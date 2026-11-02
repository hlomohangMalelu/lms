# Acceptance Test Sign-off Record

**Project:** Moderated LMS  
**Document:** AT-LMS-01  
**Build under review:** Phase 5 verification build, 2 November 2026.

## Demonstration outcomes

| Acceptance ID | Stakeholder role | Demonstrated acceptance criteria | Evidence | Demonstration result |
|---|---|---|---|---|
| AT-01 | Administrator | Create department/course, create user, register role profile, enrol student and assign lecturer | `WorkflowIntegrationTest.AdministratorSetupPersistsRoleProfilesAndCourseRelationships`; console role workflow transcript | PASS for demonstrated setup. User update operation is not implemented; therefore full account-maintenance acceptance is PARTIAL. |
| AT-02 | Lecturer | Create assignment, grant individual extension, mark submission and record feedback/audit reason | Console workflow evidence; integration workflow | PASS for demonstrated workflow. |
| AT-03 | Head of Department | Review departmental work, moderate, approve and release; ensure audit history is retained | End-to-end integration workflow; system role transcript | PASS for demonstrated workflow. |
| AT-04 | Student | View enrolled assignment, submit/resubmit, receive released feedback and not see unreleased marks | Console workflow evidence; `submission_attempts` integration test | PASS for demonstrated workflow. |

## Formal stakeholder approval

The project artefacts contain functional demonstrations and test evidence, but no external stakeholder has supplied a signed acceptance decision in this conversation. Do not treat this document as signed approval.

| Approval field | To be completed by stakeholder / supervisor |
|---|---|
| Stakeholder name and role | ______________________________ |
| Acceptance decision (Accept / Accept with conditions / Reject) | ______________________________ |
| Conditions or outstanding issues | ______________________________ |
| Signature | ______________________________ |
| Date | ______________________________ |

**Acceptance status:** Demonstration evidence recorded; formal stakeholder sign-off pending. AT-01 is partial because user-account update is not implemented.
