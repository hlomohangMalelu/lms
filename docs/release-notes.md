# Moderated LMS v1.0 — Release Notes

**Release status:** Version 1.0

## Included functionality

- Role-based authentication for Student, Lecturer, Head of Department and Administrator.
- bcrypt password hashing and password changes.
- Department, course, user and profile administration.
- Lecturer course assignment and student enrolment.
- Assignment creation, submission and resubmission workflows.
- Extension handling and configurable late-penalty strategy.
- Marking with feedback and audit reasons.
- Head-of-department moderation, approval and release of marks.
- SQLite persistence, foreign-key enforcement, parameterized SQL and audit logging.
- CMake-based C++17 build and automated test suite.

## Deployment

This release is a local console application using a local SQLite database. See `docs/deployment/installation-guide.md`, `docs/deployment/deployment-architecture.md` and the UML deployment diagram (`docs/deployment/deployment-diagram.svg`).

## Known limitations

- No web or mobile interface.
- No remote application/API server or multi-site synchronization.
- Backup and restore are operator-managed; submission file paths refer to files stored at user-supplied locations.
- The initial administrator credentials must be changed immediately after first login.
- This release does not provide automated database migration or scheduled backup facilities.

## Upgrade / data handling

Back up `data/lms.db` and submission data before replacing the executable or schema. Do not delete the data directory during an application-only upgrade. No automated database migration mechanism is documented for this release; review schema changes before upgrading an existing installation.

---

## Moderated LMS v1.1 – Maintenance release

### Change request CR1

- Lecturers can grant a deadline extension to the whole class for an assignment in a course they teach.
- Students see both the original assignment deadline and the effective deadline on their assignment list.
- An individual student extension takes precedence over a class-wide extension; if neither exists, the original assignment deadline applies.
- Existing individual extension behaviour is retained.

### Database change

- Added `class_extensions`, with one class-wide extension per assignment. Existing `extensions` rows are retained and continue to represent individual extensions.

### Regression and release status

- Unit and integration test cases cover class-extension persistence, validation, authorization and deadline precedence.
- The student-facing assignment-list display and multi-student class-extension behaviour must be verified explicitly.
- **Release gate: pending.** Do not treat this as a completed v1.1 release until the full regression suite, Ubuntu and macOS CI, coverage target, and static analysis have passed and their run links/results are recorded here.
- **Tag:** `v1.1` is to be created only after those gates pass. No release tag is asserted by this note.
