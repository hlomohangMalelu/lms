# National University of Lesotho
## Department of Mathematics and Computer Science

**Course:** CS4433 – Software Engineering  
**Project:** Moderated Learning Management System (LMS)  
**Development model:** Waterfall with V-Model verification and validation  
**Technology:** C++17, CMake, SQLite  
**Project repository:** [Moderated LMS – GitHub](https://github.com/hlomohangMalelu/lms)

# Final Project Report

## Abstract

This project developed a Moderated Learning Management System (LMS) to manage academic assignments, submissions, marking and the controlled release of student results. The system addresses the need for a structured assessment process in which lecturers assess submissions while Heads of Department (HoDs) review and approve marks before students can access them.

The application is implemented as a C++17 console program with SQLite persistence. It supports four roles: Administrator, Lecturer, Head of Department and Student. Key features include role-based access control, assignment management, individual and whole-class deadline extensions, submission and resubmission history, late penalties, marking, moderation and audit logging.

Development followed a Waterfall approach, with V-Model principles guiding verification and validation. Automated tests, integration tests, console workflow tests and continuous integration were used to evaluate the implementation.

## 1. Introduction and Project Planning

The project addresses assessment-management challenges involving submission deadlines, late work, marking accountability and the confidentiality of student results. A central requirement is that marks must not become visible to students until they have been moderated, approved and released by the appropriate Head of Department.

The project was planned over seven weeks using the Waterfall development model. Work was organised into planning, requirements engineering, system design, implementation, verification and validation, configuration management, deployment and maintenance.

A feasibility assessment considered the following:

- **Technical:** C++17, CMake and SQLite provide a suitable foundation for a local application.
- **Operational:** A console interface supports use on institutional computers without requiring a browser or internet connection.
- **Economic:** Open-source tools and bundled dependencies reduce licensing and infrastructure costs.
- **Schedule:** Development was organised around the seven-week coursework schedule.
- **Legal and ethical:** Student records, passwords and assessment results require appropriate protection and controlled access.

The implemented application is designed for a local workstation or trusted school computer. Online payment, video conferencing, automated grading, plagiarism detection and web/mobile interfaces were excluded from the project scope.

## 2. Requirements Engineering

Four principal stakeholders were identified:

| Stakeholder | Main responsibility or concern |
|---|---|
| Administrator | Manages users, departments, courses and enrolments. |
| Lecturer | Creates assignments, manages extensions, marks submissions and provides feedback. |
| Head of Department | Moderates and approves departmental marks before release. |
| Student | Views assignments, submits work and accesses released results. |

The requirements were shaped by the need to balance lecturer flexibility with institutional assessment controls. Lecturers can grant extensions and provide feedback, while students cannot access marks before the moderation and release process is completed.

The implemented system supports the following principal requirements:

1. Authenticate users and enforce role-based permissions.
2. Create and manage departments, courses and user profiles.
3. Assign lecturers to courses and enrol students.
4. Create assignments with instructions, deadlines and maximum marks.
5. Accept assignment submissions and permitted resubmissions.
6. Grant individual and whole-class deadline extensions.
7. Apply late penalties of 10% of the available mark per late day, up to three days.
8. Award zero for submissions more than three days late.
9. Allow lecturers to mark submissions and record feedback.
10. Allow HoDs to moderate, approve and release marks.
11. Prevent students from viewing unreleased marks.
12. Maintain an audit history of mark changes.

The submission process preserves previous attempts rather than simply replacing their records. Each attempt stores its file reference, timestamp, attempt number and submission status.

The use-case diagram is available in the repository: [View Use Case Diagram](https://github.com/hlomohangMalelu/lms/blob/main/docs/images/use_case.jpg).

## 3. System Design

### 3.1 Architecture

The system uses a layered architecture that separates user interaction, business operations and database persistence.

| Layer | Responsibility |
|---|---|
| Presentation | Console input, output and role-specific menus. |
| Service | Coordinates authentication, administration, submission, marking and moderation workflows. |
| Domain | Contains business entities and assessment rules. |
| Repository | Handles database operations and persistence. |
| Security | Provides password hashing and authorization policies. |
| Database | Stores institutional, submission, mark and audit records using SQLite. |

The separation improves maintainability and allows business rules to be tested independently of the console interface and database.

### 3.2 Object-oriented design and patterns

The application uses inheritance and polymorphism through an abstract `User` class. `Student`, `Lecturer`, `Admin` and `HeadOfDepartment` extend this class and implement role-specific permission behaviour.

Two important design patterns are used:

- **Repository pattern:** Separates persistence operations from the business logic through repository interfaces and SQLite implementations.
- **Strategy pattern:** The `ILatePenaltyStrategy` interface allows the late-penalty calculation to be implemented through `TenPercentPerDayStrategy`.

These decisions support separation of concerns, dependency inversion and easier testing.

The class diagram is available here: [View Class Diagram](https://github.com/hlomohangMalelu/lms/blob/main/docs/images/class_diagram.png).

### 3.3 Database design

SQLite provides persistent relational storage. The schema contains tables for users, students, lecturers, departments, courses, enrolments, assignments, extensions, class extensions, submissions, submission attempts, marks and audit records.

Foreign keys and uniqueness constraints help preserve data integrity. Parameterised SQL is used for user-supplied values. Audit records are append-only, with database triggers preventing their modification or deletion.

The database relationship diagram is available here: [View ERD](https://github.com/hlomohangMalelu/lms/blob/main/docs/images/erd_diagram.png).

### 3.4 Behavioural models

The submission lifecycle includes submission, late submission, marking, moderation and release. The design also documents the interactions involved in submitting work and moderating results.

- [Submission State Diagram](https://github.com/hlomohangMalelu/lms/blob/main/docs/images/state_diagram.png)
- [Activity Diagram](https://github.com/hlomohangMalelu/lms/blob/main/docs/images/activity_diagram.png)
- [Submit Assignment Sequence Diagram](https://github.com/hlomohangMalelu/lms/blob/main/docs/images/submit_assignment_sequence_diagram.png)
- [Moderate and Release Marks Sequence Diagram](https://github.com/hlomohangMalelu/lms/blob/main/docs/images/moderate_and_release_marks_sequence_diagram.png)

## 4. Implementation

The LMS was implemented using C++17 and CMake. SQLiteCpp provides the C++ interface to SQLite, while bcrypt is used for password hashing. The application stores its database locally at `data/lms.db`.

The four role-specific menus provide the main user workflows:

- **Administrator:** Creates users, departments and courses, assigns lecturers and enrols students.
- **Lecturer:** Creates assignments, views submissions, grants extensions, marks work and records feedback.
- **Head of Department:** Reviews departmental submissions, moderates marks, approves results and releases them.
- **Student:** Views enrolled courses and assignments, submits or resubmits work, and views released marks and feedback.

The late-penalty implementation deducts 10% of the assignment's maximum mark for each late day. Work submitted more than three days late is retained but receives zero when marked.

The system also implements whole-class extensions. The effective deadline follows this precedence:

1. Individual student extension.
2. Whole-class extension.
3. Original assignment deadline.

This ensures that a personal extension is not overridden by a class-wide extension.

Authentication uses bcrypt password hashes, and authorization checks restrict operations according to the user's role, course assignment and departmental responsibility.

## 5. Verification and Validation

Testing followed the V-Model by matching verification activities to the system's design and implementation artefacts.

| Testing level | Purpose | Evidence |
|---|---|---|
| Unit testing | Tests individual domain objects, services and business rules. | GoogleTest |
| Integration testing | Tests database persistence, repositories and service interactions. | SQLite-backed integration tests |
| System testing | Tests complete console workflows and error scenarios. | Automated console workflow tests |
| Acceptance testing | Evaluates the system against stakeholder workflows. | Acceptance test records |

The recorded business-logic coverage is **95.29%**, covering 587 of 616 executable lines across the domain and service layers. This exceeds the assignment's 70% target.

The detailed testing documentation records 126 passing tests in the 2 November local execution. The test plan also reports 135 tests for GitHub Actions run #74. These are different recorded test counts, so the final hosted run and its artifacts should be treated as the authoritative evidence for the corresponding CI execution.

The CI configuration includes:

- Ubuntu and macOS build-and-test jobs.
- A business-logic coverage job with a 70% minimum threshold.
- A clang-tidy static-analysis job.
- Test and analysis evidence uploaded as workflow artifacts.

The recorded testing evidence is available in the [Testing Documentation](https://github.com/hlomohangMalelu/lms/tree/main/docs/testing).

### Outstanding limitations

Testing and self-review identified two remaining implementation gaps:

- Administrator account updating is not implemented, although the broader requirement includes account creation, updating, deactivation and viewing.
- Some console marking and moderation operations still contain SQL workflow logic in `main.cpp` instead of consistently delegating to `MarkingService` and `ModerationService`.

Formal external stakeholder sign-off is also pending. These limitations are recorded rather than treated as completed requirements.

## 6. Configuration Management, Deployment and Maintenance

GitHub was used for source control and project configuration management. The documented workflow uses `main` as the stable branch, with focused feature and fix branches, meaningful commits and pull-request review. The project also includes CMake build configuration, `.clang-format`, `.clang-tidy` and a GitHub Actions workflow.

The application is deployed as a local console executable connected to a local SQLite database. It does not require a separate web server or remote database. Installation instructions and role-specific user guides are maintained in the repository.

Deployment documentation:

- [Installation Guide](https://github.com/hlomohangMalelu/lms/blob/main/docs/deployment/installation-guide.md)
- [Deployment Architecture](https://github.com/hlomohangMalelu/lms/blob/main/docs/deployment/deployment-architecture.md)
- [Deployment Diagram](https://github.com/hlomohangMalelu/lms/blob/main/docs/deployment/deployment-diagram.png)

### Change Request CR1

During maintenance, Change Request CR1 introduced whole-class deadline extensions. Previously, lecturers could grant extensions to individual students. The new requirement allows an authorised lecturer to extend an assignment deadline for all students enrolled in the associated course.

This was classified as **adaptive maintenance** because the system was modified to accommodate an additional institutional requirement.

The change affected the requirements, service logic, authorization policy, database schema, lecturer menu, student assignment display and regression tests. A new `class_extensions` table stores the class-wide deadline and reason.

Version 1.1 documents this change and its regression testing. Individual extensions retain precedence over class-wide extensions.

See [CR1 Change Request](https://github.com/hlomohangMalelu/lms/blob/main/docs/maintenance/change-request-CR1.md) and [CR1 Design Update](https://github.com/hlomohangMalelu/lms/blob/main/docs/maintenance/CR1-design-update.md).

## 7. Professional, Legal and Ethical Considerations

The LMS handles personal information and academic assessment records. Passwords are stored as bcrypt hashes rather than plain text, and authorization checks restrict access to protected operations.

Mark changes are recorded with the responsible user, timestamp, previous value, new value and reason. Append-only audit records help preserve accountability and reduce the risk of unauthorised alteration.

The system's moderation process also promotes fairness by ensuring that students do not see results before the HoD has reviewed and approved them. Late penalties are applied consistently according to the defined institutional rule.

The institution must additionally protect database backups and referenced submission files because they contain personal and academic information. Deployment should follow applicable data-protection requirements and institutional policies.

## 8. Conclusion

The Moderated LMS demonstrates the application of software engineering principles to a practical academic assessment-management problem. It provides role-based workflows, assignment submission and resubmission, deadline extensions, late-penalty calculations, marking, moderation, result release and audit logging.

The use of C++17, SQLite, object-oriented design, design patterns, automated testing and continuous integration provides a foundation for maintainable software. The recorded 95.29% business-logic line coverage demonstrates substantial automated verification.

Although account updating, complete service-layer delegation and formal stakeholder approval remain outstanding, the project provides a working implementation of the principal moderated assessment workflows. The maintenance work also demonstrates how a new requirement can be analysed, implemented and tested without changing the deployment architecture.

## References

1. **Phase 1 – Project Planning and Management.** CS4433 Software Engineering Individual Assignment, National University of Lesotho, 2026. *Project planning deliverable.*
2. **Phase 2 – Requirements Engineering.** CS4433 Software Engineering Individual Assignment, National University of Lesotho, 2026. *Requirements and use-case deliverable.*
3. **Phase 3 – System Design.** CS4433 Software Engineering Individual Assignment, National University of Lesotho, 2026. *Architecture, UML and database-design deliverable.*
4. Sommerville, I. (2016). *Software Engineering* (10th ed.). Pearson.
5. ISO/IEC. (2017). *ISO/IEC 14882:2017 – Programming Languages – C++.*
6. SQLite. *SQLite Documentation.* https://www.sqlite.org/docs.html
7. CMake. *CMake Documentation.* https://cmake.org/documentation/
8. OWASP. *Password Storage Cheat Sheet.* https://cheatsheetseries.owasp.org/cheatsheets/Password_Storage_Cheat_Sheet.html
9. GoogleTest. *GoogleTest Documentation.* https://google.github.io/googletest/
10. Malelu, H. (2026). *Moderated Learning Management System – Source Code and Project Documentation.* GitHub. https://github.com/hlomohangMalelu/lms

---

**Repository note:** The current repository tree contains the UML images and the later testing, deployment, quality and maintenance documents. Separate Phase 1, Phase 2 and Phase 3 report files were not located in the repository tree, so those are listed as internal coursework deliverables rather than linked to invented paths.
