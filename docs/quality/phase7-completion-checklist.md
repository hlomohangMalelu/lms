# Phase 7 Completion Checklist — Deployment and Documentation

| Requirement | Deliverable | Status |
|---|---|---|
| Deployment architecture | `docs/deployment/deployment-architecture.md` | Documented; matches local console + SQLite implementation |
| Installation instructions | `docs/deployment/installation-guide.md` | Drafted for Linux/macOS and test setup |
| Administrator manual | `docs/user-guides/administrator-guide.md` | Drafted |
| Lecturer manual | `docs/user-guides/lecturer-guide.md` | Drafted |
| Student manual | `docs/user-guides/student-guide.md` | Drafted |
| Head of Department manual | `docs/user-guides/head-of-department-guide.md` | Drafted |
| Version 1.0 release notes | `docs/release-notes.md` | Drafted; release/tag remains gated by Phase 6 hosted checks |
| Fresh-machine installation test | Install on clean Linux/macOS host | Pending |
| Backup and restore test | Verify database and submission data restore | Pending |
| Final release tag | Annotated `v1.0.0` tag | Pending Phase 6 CI/review gate |

## Exit criteria

1. Review all guides against the final menu prompts and actual runtime behaviour.
2. Perform a clean installation and first-login/password-change test.
3. Verify backup and restore using a disposable test database and submission directory.
4. Confirm Phase 6 hosted CI and review gates.
5. Create the annotated `v1.0.0` tag only after the preceding checks pass.
