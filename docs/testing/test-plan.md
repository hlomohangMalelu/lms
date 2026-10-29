# Verification and Validation Test Plan

**Project:** Moderated Learning Management System (LMS)  
**Version:** 1.0 (working test plan)  
**Test framework:** GoogleTest, CMake and CTest  
**Status:** Planned strategy; actual execution results and measured coverage must be recorded separately.

## 1. Purpose and scope

This plan defines how the LMS will be verified against its requirements and design. It follows the V-Model: unit tests verify individual classes and functions; integration tests verify collaboration between components and SQLite persistence; system tests verify complete workflows; and acceptance tests verify that the workflows meet the school stakeholders' needs.

The scope includes authentication and role permissions, user/course/assignment data, extensions, submission and resubmission, late penalties, marking and feedback, moderation and release, and audit records. Tests should also check invalid input, unavailable or invalid records, persistence failures where practical, and prevention of unauthorised access.

Out-of-scope features from the assignment, such as online payment, video conferencing, automated grading, plagiarism detection, and web/mobile interfaces, are not tested.

## 2. Test levels and V-Model mapping

| V-Model test level | Artefacts verified | Approach and key focus | Entry criteria | Exit criteria |
|---|---|---|---|---|
| **Unit** | Domain class design, business rules, validation and service logic; corresponding source files and requirements | GoogleTest tests for classes/functions in isolation. Cover valid, invalid and boundary inputs; use fakes/mocks where appropriate. | Source compiles; unit test target is configured; expected behaviour is defined by requirements/design. | All planned unit tests executed; no unexplained failures; critical business rules and exception paths covered; failures logged. |
| **Integration** | Repository interfaces and SQLite implementations; database schema and seed data; interactions between services, repositories, password hasher and database | GoogleTest tests against a temporary/test database. Verify reads/writes, constraints, transaction behaviour, state persistence and audit records. | Relevant unit-tested components exist; test database can be created independently; test data is isolated from real user data. | All planned integration tests executed; persisted results and constraints verified; database cleaned up or isolated; failures logged. |
| **System** | Complete built LMS application and end-to-end use cases, including role-specific menus and workflows | Run the application/build and exercise workflows from login through the relevant final state. Include negative/exception flows and verify database effects. | Build succeeds; required schema/seed data is available; test accounts and scenario data are prepared; unit/integration blockers are resolved. | Every planned system scenario has a recorded result; no open critical/high-severity defect prevents core workflows; evidence and defects are recorded. |
| **Acceptance** | User requirements, use-case specifications and stakeholder acceptance criteria | Walk through realistic scenarios for Student, Lecturer, Head of Department (HoD) and Administrator. Confirm the system follows school rules and users can complete their permitted tasks. | A demonstrable build is available; acceptance scenarios and expected outcomes are agreed; representative role accounts and data exist. | Each acceptance criterion is marked pass/fail with evidence; stakeholder-facing failures are logged; release acceptance is documented by the project owner/supervisor. |

## 3. Responsibilities and test environment

The student developer prepares and runs the automated tests, reviews failures, records defects, and reports measured results. Acceptance scenarios should be reviewed against the assignment's stakeholder requirements and use cases.

Use a supported C++17 compiler, CMake, SQLite, and the repository's GoogleTest setup. Run tests through CTest so the same commands can be used locally and in CI. Keep test databases and generated files separate from real application data. Tests should be repeatable and should not depend on a particular machine-specific path or pre-existing database state.

Typical commands from the repository root:

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

These commands describe the intended procedure; their success must be confirmed by running them and retaining the output.

## 4. Test design techniques

### 4.1 Equivalence partitioning

Divide inputs into classes expected to behave the same, then test representative values from each class.

| Feature | Equivalence partitions | Expected behaviour to verify |
|---|---|---|
| Login | Existing active user with correct password; wrong password; unknown username; inactive user; empty input | Only valid active credentials authenticate; invalid credentials are rejected without revealing password data. |
| Role permissions | Action permitted for the user's role; action not permitted; missing/invalid user context | Permitted actions succeed; prohibited actions are rejected. Check permissions in the service/business layer, not only by hiding menu options. |
| Deadline | Before deadline; exactly at deadline; after deadline | On-time and late classifications match the agreed deadline rule. Exact equality must be tested explicitly. |
| Submission | First submission; resubmission before deadline; resubmission after deadline; invalid assignment/student; empty or invalid submission data | Valid actions update the expected submission state; invalid actions fail safely and do not corrupt existing records. |
| Extension | Valid extension; expired/invalid extension; extension for another student; extension outside the applicable assignment/course | Only applicable, authorised extensions affect the effective deadline. |
| Mark and feedback | Valid mark range; negative mark; mark above maximum; missing feedback if required; mark on an invalid lifecycle state | Marks are validated against the assignment maximum and invalid transitions are rejected. Feedback and mark are persisted as specified. |
| Moderation/release | Mark awaiting moderation; approved mark; rejected/changed mark; release before approval; already released mark | Only authorised HoD actions can moderate/release; students cannot view unreleased marks. |
| Persistence | Existing record; missing record; duplicate unique value; violated foreign key/constraint; database error | Repository results and exceptions are handled consistently; data integrity is preserved. |

### 4.2 Boundary value analysis

Test values immediately below, at, and immediately above each important boundary. For date/time rules, use deterministic timestamps and define the unit of lateness (for example, calendar days or 24-hour periods) consistently with the requirements and implementation. Do not let a test depend on the machine's current time.

| Boundary | Values/scenarios to test | Purpose |
|---|---|---|
| Deadline | One smallest supported time unit before; exactly at; one smallest supported time unit after | Detect off-by-one deadline classification errors. |
| Late penalty: 10% per day, maximum three days | 0 days late; 1 day; 2 days; 3 days; just beyond 3 days | Verify no penalty on time, 10%, 20%, 30% deductions, and zero after the permitted late period, according to the stated rule. |
| Maximum mark | 0; 1; maximum minus 1; maximum; maximum plus 1 | Check valid range and reject out-of-range marks. Include the assignment's configured maximum. |
| Extension deadline | Just before extended deadline; exactly at it; just after it | Ensure an extension changes the effective deadline only as specified. |
| Required text fields | Empty; whitespace-only; minimum accepted length; maximum accepted length; over maximum if limits exist | Check validation and safe error handling. Apply only where the implementation defines a length limit. |
| Unique identifiers | New identifier; duplicate username/student number/course key where uniqueness applies | Ensure duplicate records are rejected without damaging existing data. |

**Late-penalty rule:** the assignment states that late work loses 10% of the available mark for each day late, for up to three days, and receives zero after that. Tests must confirm the implementation follows this policy and clarify how the day count is calculated. For example, with a maximum mark of 100, expected deductions are 10, 20 and 30 marks for 1, 2 and 3 days late; more than 3 days late receives zero. If the implementation uses a different interpretation of “day”, document and resolve it before treating the tests as final.

## 5. Planned test cases

Test IDs should be used in test names, system-test records and the requirements traceability matrix.

| ID | Level | Scenario | Expected result |
|---|---|---|---|
| UT-01 | Unit | Calculate penalty for on-time submission | No late deduction. |
| UT-02 | Unit | Calculate penalty for 1, 2 and 3 days late | Deduction is 10%, 20% and 30% of available marks respectively. |
| UT-03 | Unit | Calculate penalty after 3 days late | Result follows the stated zero-mark rule. |
| UT-04 | Unit | Test submission lifecycle transitions | Allowed transitions succeed; invalid transitions are rejected. |
| UT-05 | Unit | Validate mark below zero, at limits and above maximum | Valid values are accepted; invalid values are rejected. |
| UT-06 | Unit | Check role-specific permissions | Each role is restricted to its permitted actions. |
| IT-01 | Integration | Save and retrieve a submission using SQLite repository | Retrieved fields match the saved submission. |
| IT-02 | Integration | Attempt duplicate or constraint-violating database write | Write is rejected and existing data remains intact. |
| IT-03 | Integration | Save a mark change and its audit record | Mark and audit details persist and identify who/when/why as required by the design. |
| IT-04 | Integration | Restart/reopen database after a write | Committed data remains available. |
| ST-01 | System | Student logs in, views relevant assignments and submits before deadline | Submission is recorded as on time and can be retrieved. |
| ST-02 | System | Student resubmits before deadline | Latest valid submission is saved according to the resubmission rule. |
| ST-03 | System | Student submits late, including beyond the permitted late period | Correct late status/penalty is applied; invalid or too-late cases follow the stated rule. |
| ST-04 | System | Lecturer marks submission and provides feedback | Mark and feedback are saved; student cannot see the mark before release. |
| ST-05 | System | HoD reviews/moderates and releases marks | Only authorised HoD can moderate/release; released marks become visible to students. |
| ST-06 | System | User attempts an action outside their role | Access is denied and no unauthorised data change occurs. |
| ST-07 | System | Invalid input, missing record or database failure occurs | Error is handled without an uncontrolled crash or partial invalid update. |
| AT-01 | Acceptance | Administrator creates/manages users, departments/courses, lecturer assignments and enrolments | Administrator can complete permitted management tasks and invalid data is rejected. |
| AT-02 | Acceptance | Lecturer creates assignment, grants an individual extension, marks and writes feedback | Lecturer's permitted workflow works and data persists. |
| AT-03 | Acceptance | HoD checks department marks and releases approved marks | Marks remain hidden until moderation/release, matching the principal's requirement. |
| AT-04 | Acceptance | Student views enrolled work, submits/resubmits, then views released mark and feedback | Student can complete the permitted workflow and cannot access unreleased marks. |

These are planned scenarios, not claims that every case currently has an automated test. Where a feature is not implemented, record it as blocked/not applicable with a reason rather than reporting a pass.

## 6. Execution, evidence and defect management

1. Build from a clean build directory using the documented CMake procedure.
2. Run the automated suite with CTest and save the complete command output as test evidence.
3. Run system and acceptance scenarios against a known test database and record the starting data, steps, expected result, actual result, pass/fail and evidence.
4. Record every observed defect in `docs/testing/defect-log.md`, including a unique ID, severity, reproduction steps, cause, fix, and retest result.
5. After a fix, rerun the failing test and relevant regression tests. Do not close a defect until the retest result is recorded.
6. Report actual test counts and outcomes from the run. Do not describe planned tests as executed tests.

Suggested severity levels:

- **Critical:** data loss/corruption, security bypass, or core workflow unusable.
- **High:** major required workflow fails or an unauthorised user can perform a protected action.
- **Medium:** required behaviour is incorrect but a workaround exists.
- **Low:** minor usability, wording, or non-critical behaviour issue.

## 7. Coverage target and reporting

The assignment sets a target of at least **70% line coverage of the business-logic layer** and requires the actual figure to be reported. Coverage must be measured from an instrumented build and test run; it must not be estimated from the number of tests.

The coverage report should identify:

- the tool and compiler/build configuration used;
- which source directories/files count as business logic and which generated/third-party files are excluded;
- total executable lines, lines executed, and the resulting line-coverage percentage;
- important uncovered branches or business rules and the planned follow-up tests.

Record the measured percentage and attach/link the generated report after running coverage. Until that measurement is completed, coverage status is **not yet measured**.

## 8. Exit review

Phase 5 can be considered ready for review when:

- the four V-Model levels and their entry/exit criteria are documented;
- automated unit and integration tests run in CI;
- deadline and late-penalty boundaries are tested;
- system and acceptance scenarios include negative/exception flows and all four roles;
- actual test results and actual business-logic line coverage are recorded;
- defects include severity, cause, fix and retest result;
- relevant test cases are linked to requirements/use cases in the traceability matrix.

