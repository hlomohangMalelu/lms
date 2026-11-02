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
- Automated CR1 regression coverage includes class-extension persistence, validation, authorization, precedence, multi-student behaviour, assignment-list deadline selection and fallback to the original deadline.

### Validation record (2026-11-02)

- Local validation: the project owner reports that the full local test suite passes.
- GitHub Actions run [#74](https://github.com/hlomohangMalelu/lms/actions/runs/41477729251) on commit `eaafba9e31865a6cfc4d5747aa3be02d0b178b35`: **all four jobs passed**.
- Ubuntu and macOS: CTest passed **135/135 tests** on each platform.
- Business-logic coverage: the coverage job passed its configured **70% minimum line-coverage gate**. The exact measured percentage was not emitted in the job log inspected for this record; consult the uploaded `lms-coverage-Linux` artifact before publishing a numeric percentage.
- Static analysis: the clang-tidy job completed successfully and uploaded its report. It reported non-fatal findings (including readability/style, magic-number and reference-member checks); review and triage them before treating the report as warning-free.
- PR [#21](https://github.com/hlomohangMalelu/lms/pull/21) was merged to `main` on 2026-11-02.
- **Outstanding release gate:** manually demonstrate the lecturer whole-class extension flow and the student assignment-list display (including original/effective deadline values) using the documented acceptance scenarios, then record tester/date/evidence in the system/acceptance execution records. Formal stakeholder sign-off also remains pending as stated in the test plan.
- **Tag:** `v1.1` has not been created. Create it only after the manual acceptance evidence and sign-off are recorded.
