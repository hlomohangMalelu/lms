# Integration Test Execution Record

**Project:** Moderated LMS  
**Execution date:** 2 November 2026  
**Build:** CMake Debug, C++17, GCC 14.2.0.  
**Database:** In-memory SQLite database initialized from the complete `db/schema.sql`; all records are disposable.  
**Command:** `ctest --test-dir build-coverage --output-on-failure`.

## Executed integration cases

| ID | Integration test case | Result | Evidence |
|---|---|---|---|
| IT-01 | Student submission → lecturer marking → HoD moderation → approval → release; submission, mark record and audit data persist | PASS | `WorkflowIntegrationTest.AuthorizedSubmissionMarkModerationApprovalAndReleasePersistEndToEnd` |
| IT-02 | Individual extension persists and changes the effective deadline | PASS | `WorkflowIntegrationTest.ExtensionIsPersistedAndUsedWhenCalculatingSubmissionLateness` |
| IT-03 | File metadata constraint failure rolls back submission and file rows | PASS | `WorkflowIntegrationTest.FailedSubmissionFilePersistenceRollsBackSubmissionRow` |
| IT-04 | Admin creates department, course, accounts, role profiles, enrolment and lecturer assignment | PASS | `WorkflowIntegrationTest.AdministratorSetupPersistsRoleProfilesAndCourseRelationships` |
| IT-05 | Deadline equality is on time; one second after is late | PASS | `WorkflowIntegrationTest.DeadlineBoundaryIsInclusiveAndOneSecondAfterIsLate` |
| IT-06 | Resubmission preserves the previous attempt and updates the current file pointer | PASS | `WorkflowIntegrationTest.ResubmissionPreservesEarlierAttemptHistory` |
| IT-07 | Extension deadline equality is on time; one second after is late | PASS | `WorkflowIntegrationTest.ExtensionDeadlineIsInclusiveAndOneSecondAfterIsLate` |
| IT-08 | Login completes within the SRS two-second target with 800 student accounts stored (198 ms in the recorded run) | PASS | `WorkflowIntegrationTest.LoginMeetsTwoSecondTargetWithEightHundredStudentAccounts` |
| IT-09 | More-than-three-days-late work is stored and marked zero | PASS | `WorkflowIntegrationTest.FourDaysLateSubmissionIsStoredAndReceivesZeroAfterMarking` |
| IT-10 | Active-session database write failure rolls back all submission-related rows | PASS | `WorkflowIntegrationTest.ActiveSessionDatabaseWriteFailureRollsBackAllSubmissionRows` |
| IT-11 | Cross-course unauthorized submission is rejected without database changes | PASS | `WorkflowIntegrationTest.UnauthorizedCrossCourseSubmissionLeavesDatabaseUnchanged` |

## Result

The current clean Debug build and automated suite passed **126/126 tests**, including unit, SQLite repository/security, service integration and the black-box console/system test. The measured business-logic line coverage is **95.29% (587/616 executable lines)**; see `coverage-report.md`.

All tests use isolated in-memory or temporary SQLite data. The system test additionally launches the real console executable against disposable databases. Automated integration tests do not replace external stakeholder acceptance sign-off.
