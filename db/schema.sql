PRAGMA foreign_keys = ON;

CREATE TABLE departments (
    id    INTEGER PRIMARY KEY,
    name  TEXT NOT NULL UNIQUE
);

CREATE TABLE courses (
    id             INTEGER PRIMARY KEY,
    code           TEXT NOT NULL UNIQUE,
    title          TEXT NOT NULL,
    department_id  INTEGER NOT NULL REFERENCES departments(id) ON DELETE RESTRICT
);


CREATE TABLE users (
    id             INTEGER PRIMARY KEY,
    username       TEXT NOT NULL UNIQUE COLLATE NOCASE,
    password_hash  TEXT NOT NULL,          -- bcrypt hash; the salt is embedded in the hash string
    full_name      TEXT NOT NULL,
    role           TEXT NOT NULL CHECK (role IN ('STUDENT', 'LECTURER', 'HOD', 'ADMIN')),
    active         INTEGER NOT NULL DEFAULT 1 CHECK (active IN (0, 1))
);

CREATE TABLE students (
    user_id     INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE RESTRICT,
    student_no  TEXT NOT NULL UNIQUE
);

CREATE TABLE lecturers (
    user_id   INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE RESTRICT,
    staff_no  TEXT NOT NULL UNIQUE
);

CREATE TABLE heads_of_department (
    user_id        INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE RESTRICT,
    department_id  INTEGER NOT NULL UNIQUE REFERENCES departments(id) ON DELETE RESTRICT
);


CREATE TABLE enrolments (
    student_id  INTEGER NOT NULL REFERENCES students(user_id) ON DELETE RESTRICT,
    course_id   INTEGER NOT NULL REFERENCES courses(id) ON DELETE RESTRICT,
    PRIMARY KEY (student_id, course_id)
);

CREATE TABLE course_lecturers (
    lecturer_id  INTEGER NOT NULL REFERENCES lecturers(user_id) ON DELETE RESTRICT,
    course_id    INTEGER NOT NULL REFERENCES courses(id) ON DELETE RESTRICT,
    PRIMARY KEY (lecturer_id, course_id)
);


CREATE TABLE assignments (
    id            INTEGER PRIMARY KEY,
    course_id     INTEGER NOT NULL REFERENCES courses(id) ON DELETE RESTRICT,
    created_by    INTEGER NOT NULL REFERENCES lecturers(user_id) ON DELETE RESTRICT,
    title         TEXT NOT NULL,
    instructions  TEXT NOT NULL DEFAULT '',
    deadline      TEXT NOT NULL CHECK (datetime(deadline) IS NOT NULL),
    max_mark      REAL NOT NULL CHECK (max_mark > 0)
);

CREATE TABLE extensions (
    id             INTEGER PRIMARY KEY,
    assignment_id  INTEGER NOT NULL REFERENCES assignments(id) ON DELETE RESTRICT,
    student_id     INTEGER NOT NULL REFERENCES students(user_id) ON DELETE RESTRICT,
    granted_by     INTEGER NOT NULL REFERENCES lecturers(user_id) ON DELETE RESTRICT,
    new_deadline   TEXT NOT NULL CHECK (datetime(new_deadline) IS NOT NULL),
    reason         TEXT NOT NULL,
    UNIQUE (assignment_id, student_id)
);


CREATE TABLE submissions (
    id               INTEGER PRIMARY KEY,
    assignment_id    INTEGER NOT NULL REFERENCES assignments(id) ON DELETE RESTRICT,
    student_id       INTEGER NOT NULL REFERENCES students(user_id) ON DELETE RESTRICT,
    status           TEXT NOT NULL CHECK (status IN ('SUBMITTED', 'LATE', 'MARKED', 'MODERATED', 'RELEASED')),
    days_late        INTEGER NOT NULL DEFAULT 0 CHECK (days_late >= 0),
    penalty_fraction REAL NOT NULL DEFAULT 0 CHECK (penalty_fraction BETWEEN 0 AND 1),
    raw_mark         REAL CHECK (raw_mark IS NULL OR raw_mark >= 0),
    final_mark       REAL CHECK (final_mark IS NULL OR final_mark >= 0),
    feedback         TEXT,
    released_at      TEXT CHECK (released_at IS NULL OR datetime(released_at) IS NOT NULL),
    released_by      INTEGER REFERENCES users(id) ON DELETE RESTRICT,
    UNIQUE (assignment_id, student_id),
    CHECK (status <> 'RELEASED' OR (released_at IS NOT NULL AND released_by IS NOT NULL))
);



CREATE TABLE audit_log (
    id             INTEGER PRIMARY KEY,
    submission_id  INTEGER NOT NULL REFERENCES submissions(id) ON DELETE RESTRICT,
    changed_by     INTEGER NOT NULL REFERENCES users(id) ON DELETE RESTRICT,
    changed_at     TEXT NOT NULL DEFAULT (strftime('%Y-%m-%d %H:%M:%S', 'now'))CHECK (datetime(changed_at) IS NOT NULL),
    old_value      REAL,                   -- NULL for the first mark
    new_value      REAL NOT NULL,
    reason         TEXT NOT NULL CHECK (length(trim(reason)) > 0)
);


CREATE TRIGGER audit_log_no_update BEFORE UPDATE ON audit_log
BEGIN
    SELECT RAISE(ABORT, 'audit_log is append-only');
END;

CREATE TRIGGER audit_log_no_delete BEFORE DELETE ON audit_log
BEGIN
    SELECT RAISE(ABORT, 'audit_log is append-only');
END;


-- -------------------------indexes--------------------------------------- 
CREATE INDEX idx_courses_department        ON courses(department_id);
CREATE INDEX idx_enrolments_course         ON enrolments(course_id);
CREATE INDEX idx_course_lecturers_course   ON course_lecturers(course_id);
CREATE INDEX idx_assignments_course        ON assignments(course_id);
CREATE INDEX idx_extensions_student        ON extensions(student_id);
CREATE INDEX idx_submissions_assign_status ON submissions(assignment_id, status);
CREATE INDEX idx_submissions_student       ON submissions(student_id);
CREATE INDEX idx_audit_submission          ON audit_log(submission_id);
