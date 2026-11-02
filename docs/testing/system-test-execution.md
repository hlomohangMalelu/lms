# System Test Execution Record

**Project:** Moderated LMS  
**Execution date:** 2 November 2026  
**Build:** C++17 Debug, GCC 14.2.0, CMake 3.31.6.  
**Database:** Disposable SQLite databases under temporary directories; no production data used.  
**Automated console harness:** `tests/system/ConsoleWorkflowTest.py`, registered as `ConsoleWorkflowSystemTest` in CTest.

## Executed system scenarios

| Scenario | Outcome | Actual result / evidence |
|---|---|---|
| ST-01 Authentication | PASS for tested paths | First-run administrator and active role login succeeded; incorrect credentials and inactive account were rejected. Evidence: `evidence/ST-01-auth-smoke.txt`, `evidence/ST-01-inactive-and-db-failure.txt`. |
| ST-02 Role permissions | PASS for tested paths | Student cross-course submission, lecturer cross-course marking, HoD cross-department moderation, inactive actor and non-admin administrative attempts were denied. SQLite-backed tests confirm denied writes leave records unchanged. |
| ST-03 Student submission/resubmission | PASS | Real console accepted two submissions for the same assignment. `submission_attempts` retained attempt 1 and 2; the latest file reference pointed to attempt 2. Evidence: `ConsoleWorkflowSystemTest`. |
| ST-04 Deadline and late work | PASS for tested paths | Fixed-time service integration tests covered just before, exact deadline, one second after and extension deadline boundaries. Console tests verified a one-day deduction of 10% of maximum mark and accepted work more than three days late with a zero final mark. |
| ST-05 Lecturer marking | PASS for tested paths | Lecturer created an assignment and marked an authorized submission. Mark and audit row persisted. Over-maximum and missing-submission paths were rejected in the recorded role workflow. |
| ST-06 HoD moderation and release | PASS for demonstrated workflow | HoD moderated, approved and released a mark. Submission state, mark record and audit history were verified in the end-to-end integration workflow. |
| ST-07 Cross-department access | PASS | HoD from another department was denied; target submission and audit rows remained unchanged. |
| ST-08 Database failures | PASS for tested paths | Startup with `data` as a regular file returned a non-zero exit with a fatal diagnostic. An active-session trigger simulated a database write failure; the console displayed the error and the transaction left no submission, file or attempt rows. Evidence: `ConsoleWorkflowSystemTest`. |

## Database and workflow verification

- Full authorized workflow: student submission → lecturer marking → HoD moderation → approval → release.
- Extension workflow: authorised lecturer extension changed the effective deadline.
- More than three days late: submission was accepted, stored as `LATE`, and the marking workflow assigned a final mark of zero.
- Resubmission: both attempt records were retained; the current submission/file reference reflected the latest attempt.
- Failure atomicity: an injected SQLite write failure did not leave partial submission rows.
- Authorization: cross-course and cross-department denials did not modify protected assessment records.

## Remaining system-test limitations

- The console's mark/moderation paths still contain SQL workflow logic in `main.cpp` rather than consistently delegating to `MarkingService` and `ModerationService`; service-level integration tests separately verify those service contracts. This is recorded as an architecture/implementation issue, not concealed as complete layer wiring.
- Full manual exploratory testing on a second operating system (macOS) has not been performed.
- The console system harness is automated and uses disposable databases; it does not constitute external stakeholder sign-off.
