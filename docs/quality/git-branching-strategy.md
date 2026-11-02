# Git branching and configuration management strategy

## Repository branches

- `main`: stable, reviewed code suitable for a tagged release.
- `develop`: integration branch for completed features (if the team workflow needs one).
- `feature/<short-description>`: one planned feature or deliverable.
- `fix/<short-description>`: one defect correction.
- `docs/<short-description>`: documentation-only changes.

For a solo coursework repository, `main` plus short-lived feature/fix branches is sufficient. `develop` is optional; do not create process overhead merely to have another branch.

## Change workflow

1. Start from an up-to-date `main` and create a focused branch, e.g. `feature/phase6-quality-records`.
2. Make small, logically grouped commits. Use imperative messages such as `Add macOS CI build` or `Document self-review findings`.
3. Run the documented CMake build and CTest suite locally before pushing.
4. Push the branch and open a pull request to `main` (or merge through a documented self-review when working alone and PRs are unavailable).
5. Review the diff, CI result and test evidence; resolve failures before merging.
6. Tag only a verified release commit, using annotated tags such as `v1.0.0`.

## Commit message convention

Use `Add`, `Fix`, `Update`, `Remove`, or `Document` followed by a concise object. Keep unrelated changes in separate commits. Do not commit generated build output, local databases, coverage data or credentials.

## Reproducibility

The project uses CMake and C++17. SQLiteCpp, bcrypt and GoogleTest source are vendored so CI and school lab builds do not depend on downloading C++ dependencies at configure time. SQLite development headers/libraries and standard build tools are required on the host.
