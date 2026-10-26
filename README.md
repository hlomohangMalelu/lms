# Moderated Learning Management System

A C++17 console LMS backed by SQLite/SQLiteCpp and bcrypt password hashing.

## Implemented workflows

- **Authentication:** active-user login, bcrypt password verification, and password changes that verify the current password and require at least 12 characters for the new password.
- **Administrator:** create departments and courses; create users for all supported roles; register student/lecturer profiles; assign heads of department; assign lecturers to courses; enrol students; deactivate users; list users/courses.
- **Lecturer:** view assigned courses; create assignments with instructions, deadlines and maximum marks; view course submissions and file references; mark submissions with feedback and audit reasons; grant/update student extensions.
- **Student:** view enrolled courses and assignments; submit/resubmit a file reference within the deadline or the configured three-day late window; view released marks and feedback.
- **Head of Department:** view marked submissions within the HOD's department; moderate marks and record notes; approve moderated marks; release approved marks to students.
- **Persistence/security:** parameterized SQL for user-entered values, bcrypt hashes, SQLite foreign keys, unique constraints, append-only audit-log triggers, and transactions around submission updates and console mark/moderation workflows.
- **Domain/service code:** domain models, repositories, marking/moderation services, and a late-penalty strategy are retained in the project.

## Build and run (Fedora/Linux)

Dependencies: a C++17 compiler, CMake, make, and SQLite development headers. SQLiteCpp and bcrypt are vendored. GoogleTest is fetched by CMake when tests are enabled, which requires network access unless already cached.

```bash
cmake -S . -B build -DLMS_BUILD_TESTS=OFF
cmake --build build -j$(nproc)
./build/lms
```

Run the executable from the project root so it can load `db/schema.sql`. The database is created at `data/lms.db`.

### First login

On a fresh database the app creates this development administrator:

- Username: `admin`
- Password: `ChangeMe123!`

Change this password immediately after login. These are development bootstrap credentials, not production credentials. User creation requires an initial password of at least 12 characters.

## Test suite

```bash
cmake -S . -B build-tests -DLMS_BUILD_TESTS=ON
cmake --build build-tests -j$(nproc)
ctest --test-dir build-tests --output-on-failure
```

The existing unit tests cover domain classes and selected repository behavior. The application was built successfully with tests disabled; the full test suite and a complete multi-role end-to-end acceptance run have not been verified as part of this package build.

## Important operational notes

- Submission paths are stored as file references; the console does not upload or copy file bytes. In a deployed system, add a managed file-storage layer that validates paths and enforces file-size/type limits.
- The late penalty used by the console marker is 10% per day late, capped at 100%. The submission service allows up to three days late after the effective deadline; extensions replace the assignment deadline for that student.
- A lecturer can only mark submissions from courses assigned to them. HOD moderation, approval and release are scoped to the department attached to their HOD profile.
- This is a local educational console app. Production deployment should add stronger operational controls, backups, password-reset flows, structured logging, and automated integration tests.
