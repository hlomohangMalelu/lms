# Moderated Learning Management System — Test Plan

**Document:** TP-LMS-01  
**Version:** 1.2  
**Status:** Strategy, automated tests and execution records maintained together. Outstanding requirement/architecture gaps and external acceptance sign-off are listed explicitly in `defect-log.md` and `acceptance-signoff.md`.

## 1. Purpose and scope

This plan defines verification and validation for the C++17 Moderated LMS. It covers domain/business rules, SQLite persistence, role permissions, submission/marking/moderation workflows, and user-visible acceptance scenarios. The application is a local/offline console application; web/mobile interfaces and automated grading are out of scope.

The plan follows the V-Model: each test level verifies an appropriate development artefact. Automated tests use GoogleTest and are built/run through CMake, CTest and GitHub Actions.

## 2. Test levels and V-Model mapping

| V-Model level | Artefacts verified | Approach and examples | Entry criteria | Exit criteria |
|---|---|---|---|---|
| Unit | Domain classes, validation and business rules; class design and detailed method contracts | GoogleTest for submission state/data, role permissions, extensions, marks, audit records and late-penalty strategy | Source compiles; rule and expected outcome are defined; test dependencies available | All planned unit tests run; all failures investigated; business-rule boundaries covered; results recorded |
| Integration | Repository interfaces/implementations, SQLite schema and data mapping; service-to-repository interactions | GoogleTest against temporary SQLite databases; save/load/update and missing-record/error cases; audit persistence | Unit-tested domain types available; schema and repository implementation build; isolated test database configured | Data survives save/load; updates and constraints behave as expected; errors are handled; no test data leaks between cases |
| System | Complete executable and configured components; end-to-end use-case flows and exception paths | Build executable and run documented scenarios for each role, including submit/resubmit, mark, moderate/release and invalid actions | Build succeeds; schema/seed setup is available; test accounts/data documented; integration tests pass | Every critical scenario has an observed outcome; failures are logged; no unresolved critical/high-severity defects for release recommendation |
| Acceptance | Stakeholder needs, SRS and fully dressed use cases | Role-based acceptance checklist, demonstrated manually by a tester/user; compare actual behaviour with acceptance criteria | System build is available; acceptance scenarios and expected results approved; clean test data prepared | Each Must requirement has evidence or is explicitly reported as not met; stakeholder-visible results recorded; limitations accepted/documented |

## 3. Test environment and setup

- Language/build: C++17, CMake, CTest.
- Automated framework: GoogleTest bundled in `third_party/SQLiteCpp/googletest` to avoid fetching a dependency during configuration.
- Persistence: SQLite via SQLiteCpp; tests should use temporary or in-memory databases and must not modify a developer's real `data/lms.db`.
- CI: GitHub Actions builds and runs CTest on pushes and pull requests.
- Coverage: GCC/Clang coverage instrumentation with gcovr; scope is `src/domain/` and `src/service/`, excluding third-party code. Report the measured figure, tool/version, command and date. The 70% figure is a target, not a result until measured.

Typical local commands:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DLMS_BUILD_TESTS=ON -DLMS_ENABLE_SQLITECPP=ON \
  -DLMS_ENABLE_BCRYPT=ON -DLMS_ENABLE_COVERAGE=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Coverage report (after installing `gcovr`):

```bash
gcovr --root . --filter 'src/domain/' --filter 'src/service/' \
  --txt build/coverage-summary.txt --html-details build/coverage.html
```

## 4. Test design techniques

### 4.1 Equivalence partitioning

| Feature | Valid equivalence class | Invalid/exception class | Planned evidence |
|---|---|---|---|
| Login | Existing active user with correct password | Unknown username; incorrect password; inactive user; malformed/empty input | Unit/service tests and system scenario ST-01 |
| Role access | Active user performing an action allowed to their role | User performs action outside role permissions; inactive account | Role unit tests and ST-02 |
| Submission | Enrolled student submits for an existing assignment | Unknown assignment; non-enrolled student; empty/invalid submission; missing submission on update | Domain/repository tests and ST-03 |
| Deadline/penalty | On-time submission; submission 1–3 days late | Negative late days; more than 3 days late; invalid maximum/raw mark | Late-penalty unit tests UT-30–UT-37 and ST-03 |
| Marks | Mark in inclusive range 0..maximum mark | Negative mark; mark greater than maximum; missing submission | Unit/system tests ST-04 |
| Moderation/release | HoD moderates eligible marked work and releases approved marks | Student attempts to moderate; release attempted before moderation; wrong-department record | Service/system tests ST-05; add automated coverage if service API supports it |
| Persistence | Existing record saved, retrieved and updated | Missing record, duplicate/constraint violation, database failure | Repository integration tests IT-01–IT-04 |

### 4.2 Boundary value analysis

| Input/rule | Boundary values to test | Expected behaviour |
|---|---|---|
| Submission deadline | One second before; exactly at; one second after | Before and exactly at the effective deadline are on time; one second after is late. Verified using fixed timestamps at the service/integration level. |
| Late days | -1, 0, 1, 2, 3, 4+ | Negative rejected; 0 has no penalty; 1–3 apply 10% of maximum mark per late day; 4+ is accepted and receives zero when marked. |
| Raw mark | -1, 0, maximum, maximum + 1 | Negative and over-maximum rejected; endpoints accepted. |
| Extension deadline | One second before; exactly at; one second after the approved extension deadline | Before and exactly at the extended deadline are on time; one second after is late. Verified using fixed timestamps at integration level. |
| Identifiers/records | Existing ID, missing ID, duplicate unique value | Existing record succeeds; missing record yields documented error; duplicate is rejected by validation/DB constraint. |

Where a requirement does not explicitly define inclusive deadline equality or extension edge cases, record the decision as an assumption to confirm rather than silently treating it as settled.

## 5. Automated test inventory

Automated tests are organised under `tests/unit`, `tests/integration` and `tests/system`. GoogleTest cases are discovered by CTest; the black-box console test is registered directly with CTest. The full current inventory is **126 CTest tests**.

| ID | Level | Scenario / expected result | Current status |
|---|---|---|---|
| UT-01 | Unit | Domain value objects, assignment, submission, mark record, audit log and role behaviour | Automated tests pass |
| UT-02 | Unit | Late penalty for on-time, 1–3 days late, and 4+ days late; invalid negative days and mark bounds | Automated tests pass |
| UT-03 | Unit | Authentication: valid credentials, case-insensitive username, empty/unknown/wrong credentials and inactive account | Automated tests pass |
| UT-04 | Unit | Submission validation, status transitions, resubmission restrictions and extension validation | Automated tests pass |
| UT-05 | Unit | Marking/moderation validation, approval/release state rules and audit behaviour | Automated tests pass |
| UT-06 | Unit/security | Authorization policy denies unenrolled, inactive, unassigned, wrong-role and cross-department actors | SQLite-backed tests pass |
| IT-01 | Integration | Submission, marking, moderation, approval and release persist through services, repositories and SQLite | Automated test passes |
| IT-02 | Integration | Individual extension persistence and deadline override | Automated test passes |
| IT-03 | Integration | Constraint/write failure rolls back submission transaction | Automated test passes |
| IT-04 | Integration | Admin creates department, course, users, role profiles, enrolments and lecturer assignment | Automated test passes |
| IT-05–IT-07 | Integration | Deadline/extension boundaries and resubmission attempt history | Automated tests pass |
| IT-09 | Integration | More-than-three-days-late submission persists and is marked zero | Automated test passes |
| IT-10 | Integration | Active-session database write failure leaves no partial submission data | Automated test passes |
| IT-11 | Integration | Unauthorized cross-course submission is denied without database changes | Automated test passes |
| IT-08 | Integration/performance | Login completes under two seconds with 800 student accounts stored | Automated test passes in current environment; see execution record for exact run |
| ST-01–ST-08 | System | Actual console authentication, role workflows, deadline/late behaviour, resubmission, invalid access and database exception paths | Executed paths recorded in `system-test-execution.md`; console system harness is part of CTest |
| AT-01 | Acceptance | Admin account/course setup | Demonstrated partially; account update feature remains unimplemented |
| AT-02 | Acceptance | Lecturer assignment, extension, marking and feedback | Demonstrated; formal stakeholder sign-off pending |
| AT-03 | Acceptance | HoD moderation, approval and release | Demonstrated; formal stakeholder sign-off pending |
| AT-04 | Acceptance | Student assignment view, submit/resubmit and released feedback | Demonstrated; formal stakeholder sign-off pending |

The test execution records and the exact test names are authoritative. Do not interpret a partial acceptance result as complete satisfaction of the related requirement.

## 6. Execution, defect handling and regression

1. Configure and build from a clean build directory.
2. Run `ctest --test-dir build --output-on-failure`; preserve the full console output.
3. Run the system and acceptance scenarios with documented test data; record actual results and evidence.
4. For each failure, create a defect-log row containing ID, severity, cause, fix, retest result and evidence. Do not create fictional defects to populate the log.
5. After a fix, rerun the failing test and the full regression suite.
6. Generate coverage after the test run; record the actual line percentage and include the HTML/text report as an artefact.

Severity guidance: **Critical** = data loss/security bypass/system unusable; **High** = core workflow blocked or assessment integrity compromised; **Medium** = feature impaired with workaround; **Low** = cosmetic/minor usability issue.

## 7. Exit/release criteria

- All automated tests execute and all failures are either fixed or explicitly justified and documented.
- All critical system scenarios have recorded outcomes.
- No unresolved critical or high-severity defect remains; current open items are medium-severity requirement/architecture gaps recorded in `defect-log.md`.
- Formal stakeholder acceptance sign-off is obtained before describing the system as accepted; the current record is pending.
- Coverage is measured for the business-logic layer; target is at least 70% line coverage. Any shortfall is reported honestly with a plan to address it.
- Test results, coverage report, defect log and CI run evidence are included in the Phase 5 submission.

## 8. Limitations

Automated tests do not replace external stakeholder acceptance. AT-01 is partial because administrator user-account update is not implemented; the remaining acceptance workflows have demonstration evidence but await stakeholder sign-off. The console currently contains direct SQL for marking/moderation rather than consistently delegating to the service layer; this Phase 3 alignment gap is recorded as ARCH-01. Coverage and test counts must be regenerated after any subsequent source changes.


## Alignment with Phase 3 System Design

This test strategy follows the submitted Phase 3 design: a layered architecture with Presentation, Service, Business Logic, Repository/DAO, and SQLite persistence layers. Tests should verify each layer independently where practical and verify the connections between layers at higher V-Model levels. The design names `AuthService`, `SubmissionService`, `MarkingService`, `ModerationService`, and `AdminService`; the business rules include user roles, assignments, submissions, and late-penalty calculations.

### V-Model verification mapping

| V-Model level | Phase 3 artefact verified | Main test focus | Entry criteria | Exit criteria |
|---|---|---|---|---|
| Unit | Class diagram and business-logic classes (`User`, `Course`, `Assignment`, `Submission`, `ILatePenaltyStrategy`, `TenPercentPerDayStrategy`, mark/audit entities) | Valid/invalid values, role data, submission state and penalty calculations | Class/API available; expected rules documented | All planned unit cases executed; failures resolved or logged |
| Integration | Layered architecture, repository interfaces, SQLite repositories, connection manager and audit log writer | Services use repository/DAO correctly; SQL persistence, constraints, transactions and audit data | Unit tests pass; schema available; test database isolated | Data round-trips correctly; rollback/error paths checked; no unexplained failures |
| System | Behavioural models: Submit Assignment sequence, Moderate and Release Marks sequence, submission state machine and activity diagram | Complete workflows through menus, services, business rules and SQLite | Build succeeds; integration tests pass; test data prepared | All critical happy paths and exception paths executed; defects logged |
| Acceptance | Four role menus and use cases from the LMS case study | Admin setup, lecturer assignment/extension/marking, HoD moderation/release, student submission/resubmission and viewing released marks | System build available; role-specific scenarios agreed | Each acceptance case has an observed result and evidence; unresolved critical defects disclosed |

### Design-pattern verification

- **Strategy pattern:** test `TenPercentPerDayStrategy` through the `ILatePenaltyStrategy` contract, including boundary cases. A different strategy should be usable by submission logic without changing the workflow, where supported by the implementation.
- **Repository pattern:** verify that service-facing repository interfaces can be replaced by test fakes for unit tests, and verify SQLite implementations separately in integration tests.
- **Dependency inversion:** service unit tests should avoid requiring a live SQLite database when repository fakes can isolate business rules.

### Phase 3 database artefacts and persistence checks

Use the submitted data dictionary as a conceptual reference for `User`, `Department`, `Course`, course-lecturer assignment, enrolment, `Assignment`, `Extension`, `Submission`, `Mark_Record`, and `Audit_Log`. Before executing SQL assertions, confirm the actual table and column names in `db/schema.sql`; if the implementation differs from the Phase 3 names, record the mapping rather than assuming the names are identical. Verify foreign-key constraints, parameterised data access, persistence after reconnecting, and audit fields for mark changes (actor, timestamp, old mark, new mark and reason).

### Equivalence classes and boundary values

| Area | Equivalence classes | Boundary/value cases |
|---|---|---|
| Authentication | valid active user; wrong password; unknown username; inactive user | empty username/password; correct credentials for inactive account |
| Role access | action permitted for role; action forbidden for role | each of Student, Lecturer, HoD and Administrator attempts an out-of-role action |
| Deadline | before deadline; exactly at deadline; after deadline | one instant before, exactly at, and one instant after the effective deadline |
| Late penalty | on time; 1, 2, 3 days late; over 3 days late | 0 days; 1 day; 3 days; more than 3 days; mark at 0 and maximum mark |
| Marks | valid mark; negative mark; mark above maximum; invalid/non-numeric input | 0, maximum mark, below 0, and just above maximum |
| Extensions | valid authorised extension; invalid date; unauthorised grant | revised deadline before, equal to, and after the original deadline, subject to implemented rules |
| Submission lifecycle | valid transition; invalid transition | resubmit before deadline; attempt after deadline; mark before submission; release before moderation |
| Persistence | valid foreign keys; missing/invalid related record | nonexistent user/course/assignment/submission ID; database constraint failure |

For deadline and extension tests, use a controllable clock or fixed timestamps where the implementation supports it. Do not rely on real-time sleeps. Expected results must follow the business rules and actual implementation; record any discrepancy as a design/implementation issue.


## Integration test execution

The SQLite-backed service/repository workflow integration suite and its executed results are recorded in `integration-test-execution.md`. The tests use the actual schema and cover submission, extension, marking, moderation, approval, release, authorization denial, and transaction rollback.
