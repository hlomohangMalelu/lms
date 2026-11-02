#!/usr/bin/env python3
"""Black-box console tests for late submissions, resubmission and DB failure."""
from __future__ import annotations

import re
import shutil
import sqlite3
import subprocess
import sys
import tempfile
from datetime import datetime, timedelta
from pathlib import Path


def prepare_workspace(project: Path, *, reject_file_write: bool = False) -> tuple[tempfile.TemporaryDirectory, Path]:
    temporary = tempfile.TemporaryDirectory(prefix="lms-console-system-")
    root = Path(temporary.name)
    (root / "db").mkdir()
    (root / "data").mkdir()
    shutil.copy2(project / "db" / "schema.sql", root / "db" / "schema.sql")
    database = root / "data" / "lms.db"
    connection = sqlite3.connect(database)
    connection.executescript((project / "db" / "schema.sql").read_text(encoding="utf-8"))
    if reject_file_write:
        connection.executescript(
            "CREATE TRIGGER reject_submission_file BEFORE INSERT ON submission_files "
            "BEGIN SELECT RAISE(ABORT, 'simulated disk/write failure'); END;"
        )
    connection.commit()
    connection.close()
    return temporary, root


def run_console(binary: Path, root: Path, inputs: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [str(binary)],
        cwd=root,
        input="\n".join(inputs) + "\n",
        text=True,
        capture_output=True,
        timeout=45,
        check=False,
    )


def admin_and_assignment_inputs(deadline: str) -> list[str]:
    return [
        "1", "admin", "ChangeMe123!",  # Username, then password.
        "1", "Computing",               # Create department.
        "2", "CS101", "Software Engineering", "1",  # Code, title, department id.
        "3", "student1", "Student One", "STUDENT", "StudentPass123!",
        "4", "2", "S001",             # User id, then student number.
        "8", "2", "1",                # Student user id, then course id.
        "3", "lecturer1", "Lecturer One", "LECTURER", "LecturerPass123!",
        "5", "3", "L001",             # User id, then staff number.
        "7", "3", "1",                # Lecturer user id, then course id.
        "0",                             # Admin logout.
        "1", "lecturer1", "LecturerPass123!",
        "2", "1", "Assignment One", "System test assignment", deadline, "100",
        "0",                             # Lecturer logout.
    ]


def test_startup_database_path_failure(binary: Path) -> None:
    with tempfile.TemporaryDirectory(prefix="lms-console-startup-failure-") as temporary:
        root = Path(temporary)
        (root / "data").write_text("not a directory", encoding="utf-8")
        result = subprocess.run(
            [str(binary)], cwd=root, input="", text=True,
            capture_output=True, timeout=15, check=False,
        )
        output = result.stdout + result.stderr
        if result.returncode == 0 or "Fatal error:" not in output:
            raise AssertionError(
                f"Expected safe non-zero startup failure. Exit={result.returncode}\n{output}"
            )
        print("ST-08 startup database-path failure: PASS (diagnostic and non-zero exit)")


def test_active_session_write_failure(binary: Path, project: Path) -> None:
    temporary, root = prepare_workspace(project, reject_file_write=True)
    try:
        inputs = admin_and_assignment_inputs("2099-12-10 23:59:00") + [
            "1", "student1", "StudentPass123!",
            "2", "1", "work.pdf",
            "0", "0",
        ]
        result = run_console(binary, root, inputs)
        if result.returncode != 0:
            raise AssertionError(f"Console exited unexpectedly: {result.returncode}\n{result.stdout}\n{result.stderr}")
        output = result.stdout + result.stderr
        if "simulated disk/write failure" not in output:
            raise AssertionError(f"Expected database failure diagnostic was not shown.\n{output}")
        if re.search(r"Saved submission id\s+\d+", output):
            raise AssertionError("Application reported success despite the forced database failure.")

        connection = sqlite3.connect(root / "data" / "lms.db")
        counts = {
            table: connection.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0]
            for table in ("submissions", "submission_files", "submission_attempts")
        }
        connection.close()
        if any(counts.values()):
            raise AssertionError(f"Failed transaction left partial data: {counts}")
        print("ST-08 active-session write failure: PASS (clear error, no partial rows)")
    finally:
        temporary.cleanup()


def test_one_day_late_penalty_uses_maximum_mark(binary: Path, project: Path) -> None:
    temporary, root = prepare_workspace(project)
    try:
        # Keep the elapsed interval just below 24 hours so ceiling-to-day logic yields one day.
        deadline = (datetime.now().astimezone() - timedelta(hours=23, minutes=59)).strftime("%Y-%m-%d %H:%M:%S")
        inputs = admin_and_assignment_inputs(deadline) + [
            "1", "student1", "StudentPass123!",
            "2", "1", "one-day-late.pdf",
            "0",
            "1", "lecturer1", "LecturerPass123!",
            "4", "1", "80", "One day late", "Penalty basis test",
            "0", "0",
        ]
        result = run_console(binary, root, inputs)
        if result.returncode != 0:
            raise AssertionError(f"Console exited unexpectedly: {result.returncode}\n{result.stdout}\n{result.stderr}")
        connection = sqlite3.connect(root / "data" / "lms.db")
        submission = connection.execute(
            "SELECT days_late, raw_mark, final_mark, penalty_fraction "
            "FROM submissions WHERE assignment_id = 1 AND student_id = 2"
        ).fetchone()
        connection.close()
        if not submission or submission[0] != 1 or submission[2] != 70.0 or submission[3] != 0.1:
            raise AssertionError(
                f"Expected 10 marks deducted from a 100-mark maximum, got {submission}"
            )
        print("ST-04 one-day late penalty uses 10% of maximum mark: PASS")
    finally:
        temporary.cleanup()


def test_late_zero_mark_and_resubmission_history(binary: Path, project: Path) -> None:
    temporary, root = prepare_workspace(project)
    try:
        deadline = (datetime.now().astimezone() - timedelta(days=4)).strftime("%Y-%m-%d %H:%M:%S")
        inputs = admin_and_assignment_inputs(deadline) + [
            "1", "student1", "StudentPass123!",
            "2", "1", "first-attempt.pdf",
            "2", "1", "second-attempt.pdf",
            "0",                             # Student logout.
            "1", "lecturer1", "LecturerPass123!",
            "4", "1", "80", "Late work", "System boundary test",
            "0", "0",
        ]
        result = run_console(binary, root, inputs)
        if result.returncode != 0:
            raise AssertionError(f"Console exited unexpectedly: {result.returncode}\n{result.stdout}\n{result.stderr}")
        output = result.stdout + result.stderr
        if output.count("Saved submission id") != 2:
            raise AssertionError(f"Expected two accepted attempts.\n{output}")
        if "Mark saved and audited." not in output:
            raise AssertionError(f"Expected lecturer marking confirmation.\n{output}")

        connection = sqlite3.connect(root / "data" / "lms.db")
        submission = connection.execute(
            "SELECT status, days_late, penalty_fraction, raw_mark, final_mark "
            "FROM submissions WHERE assignment_id = 1 AND student_id = 2"
        ).fetchone()
        attempts = connection.execute(
            "SELECT attempt_number, file_path FROM submission_attempts "
            "WHERE submission_id = 1 ORDER BY attempt_number"
        ).fetchall()
        connection.close()
        if not submission or submission[0] != "MARKED" or submission[1] <= 3:
            raise AssertionError(f"Unexpected late submission state: {submission}")
        if submission[2] != 1.0 or submission[4] != 0.0:
            raise AssertionError(f"More-than-three-days-late work was not reduced to zero: {submission}")
        expected_attempts = [(1, "first-attempt.pdf"), (2, "second-attempt.pdf")]
        if attempts != expected_attempts:
            raise AssertionError(f"Submission attempt history was not preserved: {attempts}")
        print("ST-03/FR-10 resubmission history and >3-day late zero mark: PASS")
    finally:
        temporary.cleanup()


def main() -> int:
    if len(sys.argv) != 3:
        print("Usage: ConsoleWorkflowTest.py <lms-binary> <project-root>", file=sys.stderr)
        return 2
    binary = Path(sys.argv[1]).resolve()
    project = Path(sys.argv[2]).resolve()
    test_startup_database_path_failure(binary)
    test_active_session_write_failure(binary, project)
    test_one_day_late_penalty_uses_maximum_mark(binary, project)
    test_late_zero_mark_and_resubmission_history(binary, project)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
