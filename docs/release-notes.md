# Moderated LMS 1.0.0 — Release Notes

**Release status:** Version 1.0.0 documentation baseline

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

This release is a local console application using a local SQLite database. See `docs/deployment/installation-guide.md` and `docs/deployment/deployment-architecture.md`.

## Known limitations

- No web or mobile interface.
- No remote application/API server or multi-site synchronization.
- Backup and restore are operator-managed.
- The initial administrator credentials must be changed immediately after first login.
- Hosted Ubuntu/macOS CI, coverage and static-analysis evidence must be confirmed before declaring the release fully verified.

## Upgrade / data handling

Back up `data/lms.db` and submission data before replacing the executable or schema. Do not delete the data directory during an application-only upgrade. No automated database migration mechanism is documented for this release; review schema changes before upgrading an existing installation.
