# Automated Test Results

**Environment:** Linux x86_64, GNU C++ 14.2.0, CMake 3.31.6, C++17, SQLite 3.46.1, GoogleTest bundled with SQLiteCpp.  
**Build configuration:** Debug; `LMS_BUILD_TESTS=ON`, `LMS_ENABLE_SQLITECPP=ON`, `LMS_ENABLE_BCRYPT=ON`.  
**Build directory used for this verification:** `build-coverage` (fresh clean build).  
**Commands:** `cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DLMS_BUILD_TESTS=ON -DLMS_ENABLE_SQLITECPP=ON -DLMS_ENABLE_BCRYPT=ON -DLMS_ENABLE_COVERAGE=ON`, `cmake --build build-coverage --parallel 2`, `ctest --test-dir build-coverage --output-on-failure`, and `python3 docs/testing/measure-coverage.py`.  
**Result recorded:** 2 November 2026 (fresh clean Debug build including SEC-01 security tests, SQLite workflow integration tests and black-box console system tests).

## Result

- Build: **PASS** (`lms` and `lms_tests` targets built successfully).
- Automated tests: **126 passed, 0 failed** (CTest reported 100% passed; includes 125 GoogleTest cases and one black-box console system test).
- Test scope: domain objects, role permissions, late-penalty rules, SQLite persistence/audit logging, authentication, submission/resubmission/extensions, administration, marking, moderation, approval and release workflows, plus SQLite-backed negative authorization checks for unenrolled students, unassigned/inactive lecturers, cross-department HoDs, inactive administrators and non-admin users.
- Coverage: **95.29% line coverage (587/616 executable lines)** across `src/domain/` and `src/service/`, measured from a clean instrumented build using `docs/testing/measure-coverage.py`. The 70% target is achieved; see `coverage-report.md`.
- System demonstrations: all four role workflows were exercised using disposable SQLite data. The console CTest harness also verifies active-session database write failure, startup failure, preserved resubmission history, and more-than-three-days-late zero marking. See `system-test-execution.md` and `evidence/console-system-test-2026-11-02.txt`.
- Acceptance: AT-02 to AT-04 were demonstrated; AT-01 is partial because account update is not implemented. Formal stakeholder sign-off remains outstanding; see `acceptance-signoff.md`.
- Open medium-severity items: administrator user-account update and console-to-service-layer alignment for marking/moderation are recorded in `defect-log.md`.

The automated result was reproduced from a clean build. Passing automated tests does not establish that all manual system/acceptance scenarios have passed. The default first-run password must be changed immediately in a real deployment.



## Integration, system and acceptance execution

Eleven SQLite-backed integration test cases (covering the listed persistence, workflow, security, boundary and performance behaviours) are recorded in `tests/integration/WorkflowIntegrationTest.cpp`; the full suite passed **126/126**, including unit/security tests and the console system test. Details are in `integration-test-execution.md`.

Console-driven system testing exercised all four roles using a disposable database. The verified workflows include administrator department/course/user/profile/enrolment setup; lecturer assignment creation, extension and marking; student submission, resubmission and released-mark visibility; HoD moderation, approval and release; and cross-course/cross-department denial. Unknown assignment, over-maximum mark and missing submission attempts were also rejected. Startup database-path failure was surfaced with a non-zero exit. See `system-test-execution.md` and `evidence/system-role-workflow-2026-11-02.txt`.

Formal stakeholder acceptance sign-off remains outstanding. Fixed-time service/integration tests cover deadline and extension boundaries; the console test covers a four-plus-day late submission. The console marking/moderation layer alignment and administrator user-account update remain documented medium-severity gaps.
