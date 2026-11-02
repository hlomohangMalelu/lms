#!/usr/bin/env python3
"""Measure GCC line coverage for src/domain and src/service from lms_tests."""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "build-coverage"
OBJECT_ROOT = BUILD / "tests/CMakeFiles/lms_tests.dir/__/src"


def main():
    if not BUILD.exists():
        raise SystemExit("Coverage build not found. Configure and build build-coverage first.")
    objects = []
    for layer in ("domain", "service"):
        layer_dir = OBJECT_ROOT / layer
        if layer_dir.exists():
            objects.extend(layer_dir.rglob("*.gcno"))
    if not objects:
        raise SystemExit("No instrumented domain/service test objects found.")

    totals = {}
    with tempfile.TemporaryDirectory(prefix="lms-gcov-") as temp:
        temp = Path(temp)
        for notes in objects:
            subprocess.run(
                ["gcov", "-b", "-c", "-o", str(notes.parent), str(notes)],
                cwd=temp, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                check=False,
            )
        for report in temp.glob("*.gcov"):
            with report.open(errors="replace") as stream:
                match = re.search(r"Source:(.*)", stream.readline())
                if not match:
                    continue
                source = Path(match.group(1).strip())
                try:
                    relative = source.relative_to(ROOT / "src")
                except ValueError:
                    continue
                if not (str(relative).startswith("domain/") or str(relative).startswith("service/")):
                    continue
                if source.suffix != ".cpp":
                    continue
                hit = executable = 0
                for line in stream:
                    match = re.match(r"\s*([^:]+):\s*(\d+):", line)
                    if not match or match.group(1).strip() == "-":
                        continue
                    count = match.group(1).strip()
                    executable += 1
                    if count not in ("#####", "====="):
                        hit += 1
                totals[str(relative)] = (hit, executable)

    covered = sum(hit for hit, _ in totals.values())
    executable = sum(total for _, total in totals.values())
    if not executable:
        raise SystemExit("No executable source lines were found.")
    print("Business-logic line coverage (src/domain + src/service)")
    print(f"Covered executable lines: {covered}/{executable}")
    print(f"Line coverage: {covered / executable * 100:.2f}%")
    print("\nPer-file results:")
    for name, (hit, total) in sorted(totals.items()):
        print(f"{hit:4}/{total:<4} {hit / total * 100:6.2f}%  {name}")


if __name__ == "__main__":
    main()
