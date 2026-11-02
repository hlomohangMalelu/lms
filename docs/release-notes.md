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

# Moderated LMS v1.1 – Whole-Class Deadline Extensions (CR1)

**Release status:** Ready for release  
**Version:** 1.1  
**Change request:** CR1 – Whole-Class Deadline Extension

## Overview

Version 1.1 introduces whole-class deadline extensions, allowing authorized lecturers to extend an assignment deadline for all students enrolled in the associated course.

This release builds on the existing individual-extension functionality and improves deadline management for lecturers and students.

## Added

- **Whole-class deadline extensions:** Lecturers can grant an extension that applies to all students enrolled in the assignment's course.
- **Lecturer authorization:** Only active lecturers assigned to the relevant course can grant extensions.
- **Deadline precedence:** The system applies deadlines in this order:
  1. Individual student extension.
  2. Whole-class extension.
  3. Original assignment deadline.
- **Student deadline visibility:** Students can view the original deadline and the effective deadline on their assignment list.
- **Regression coverage:** Integration tests cover whole-class extension behavior, multiple enrolled students, student deadline display, and fallback to the original deadline when no extension exists.

## Testing and validation

All checks have been verified successfully.

| Validation | Result |
|---|---|
| Local automated tests | Passed |
| Ubuntu CTest | 135/135 passed |
| macOS CTest | 135/135 passed |
| Coverage threshold | Passed (70% minimum) |
| clang-tidy job | Passed |
| Manual acceptance checks | Verified |
| Stakeholder sign-off | Confirmed |

**CI run:** [GitHub Actions – Run #74](https://github.com/hlomohangMalelu/lms/actions/runs/41477729251)

## Documentation

- Updated student and lecturer user guides.
- Updated CR1 change request documentation and design diagrams.
- Updated the test plan and requirements traceability.
- Recorded the CR1 implementation and validation details.

## Compatibility

The project is tested through CI on Ubuntu and macOS.

## Release decision

All implementation, automated testing, manual verification, documentation, and acceptance checks are complete. No outstanding items remain.

**Moderated LMS v1.1 is ready for release.**
