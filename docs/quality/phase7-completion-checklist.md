# Phase 7 Completion Checklist — Deployment and Documentation

## Required coursework deliverables

| Requirement | Deliverable | Review status |
|---|---|---|
| UML deployment diagram | `docs/deployment/deployment-diagram.svg` (with PNG preview and editable DOT source) | Present; diagram checked against the local console/SQLite deployment model |
| Installation guide | `docs/deployment/installation-guide.md` | Present; Linux/macOS build and first-run steps documented |
| Administrator manual | `docs/user-guides/administrator-guide.md` | Present; menu options checked against the current source |
| Lecturer manual | `docs/user-guides/lecturer-guide.md` | Present; workflow checked against the current source |
| Student manual | `docs/user-guides/student-guide.md` | Present; workflow checked against the current source |
| Head of Department manual | `docs/user-guides/head-of-department-guide.md` | Present; workflow checked against the current source |
| Release notes | `docs/release-notes.md` | Present for version `v1.0`; release tag not yet created |

## Verification and release gates

- [x] Phase 6 hosted macOS build-and-test job passed (reported GitHub Actions run).
- [x] Phase 6 hosted Ubuntu build-and-test job passed (reported GitHub Actions run).
- [x] Phase 6 business-logic coverage job passed; configured line-coverage threshold was 70%.
- [x] Phase 6 clang-tidy static-analysis job passed.
- [x] SQLite backup integrity check returned `ok`.
- [x] Restored SQLite database integrity check returned `ok`.
- [x] Original and restored database user counts both returned `5`.
- [ ] Verify restoration of the actual submission files referenced by `submission_files` and `submission_attempts`.
- [ ] Perform a clean installation and first-login/password-change test on a fresh school machine.
- [ ] Obtain any required supervisor/stakeholder acceptance sign-off.
- [ ] Create and push the annotated Git tag `v1.0` after the remaining release gates are satisfied.

## Operational notes

- The application stores submission file paths supplied by users; it does not manage a dedicated `data/submissions/` directory. Backup/restore testing must therefore include the real files at those referenced paths.
- The successful database integrity checks do not prove that referenced submission files were backed up or restored.
- The installation guide assumes the source repository includes its vendored SQLiteCpp, SQLite and bcrypt dependencies. A Git clone should initialise submodules recursively.
- The current release is a local, single-instance console application. Do not describe it as a network-hosted or browser-based LMS.
