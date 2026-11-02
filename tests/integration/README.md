# Integration Tests

`WorkflowIntegrationTest.cpp` exercises the Phase 3 layered workflow using the actual `db/schema.sql` in an in-memory SQLite database.

The integration suite covers:

- Administrator setup: department, course, users, role profiles, student enrolment and lecturer-course assignment.
- Student submission, latest file reference and persistent attempt history for resubmissions.
- Deadline and individual-extension boundaries (exactly at the deadline and one second after).
- Late work through three days and accepted work beyond three days that receives zero when marked.
- Lecturer marking, HoD moderation, approval, release and audit persistence through repositories and services.
- Authorization denial for a student outside the assignment course.
- SQLite constraint and active-session write failures with transaction rollback.
- Login performance with 800 student accounts against the SRS two-second target.

Run with `ctest --test-dir build-coverage --output-on-failure` after configuring with `-DLMS_BUILD_TESTS=ON` and `-DLMS_ENABLE_COVERAGE=ON`.
