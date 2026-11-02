# Phase 6 verification record

## Local environment

- OS: Linux (container build environment)
- Compiler: GNU C++ 14.2.0
- CMake: 3.31.6
- Language standard: C++17
- Configuration: Debug, tests enabled, vendored SQLiteCpp and bcrypt enabled

## Commands

```sh
cmake -S . -B build-phase6 -DCMAKE_BUILD_TYPE=Debug \
  -DLMS_BUILD_TESTS=ON -DLMS_ENABLE_SQLITECPP=ON \
  -DLMS_ENABLE_BCRYPT=ON -DLMS_ENABLE_COVERAGE=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build-phase6 --parallel 2
ctest --test-dir build-phase6 --output-on-failure
python3 scripts/code-metrics.py
```

## Results

- CMake configure: passed.
- Main application target `lms`: built successfully.
- Full test target: build was interrupted by the execution time limit during the final compilation units; resumed build and final CTest result are to be recorded below.
- Ubuntu and macOS hosted CI: pending push; no remote result is claimed.
- clang-tidy: local tool not installed in the build environment; CI installs and runs it. Review the uploaded CI artifact before merging.

This record deliberately distinguishes local results from checks that require GitHub Actions. Update it with the final resumed build/CTest outcome and the hosted workflow run URL after pushing.

### Final local run

- Build: passed (`lms` and `lms_tests`).
- CTest: **126/126 passed, 0 failed** (including the console workflow test).
- Metrics: see `code-metrics-2026-11-02.txt` (71 C++ source/header files; approximately 3,835 non-blank, non-comment-leading lines; highest detected complexity estimates are listed there).
- Static analysis: not executed locally because clang-tidy is not installed. The GitHub Actions static-analysis job is configured but must run on the remote repository before its result can be recorded.
- Cross-platform run: macOS job is configured but not yet executed; local verification was Linux only.

## Follow-up verification against the supplied integration archive

The original `src/menus/` role-menu structure was retained. `src/main.cpp` remains the application entry point and dispatches to `adminMenu`, `studentMenu`, `lecturerMenu`, and `hodMenu`; it does not contain role-menu implementations.

During integration, menu construction was updated to inject the existing `SQLiteAuthorizationPolicy` into `AdminService` and `SubmissionService`, matching the current service interfaces.

A black-box system test exposed a lecturer marking defect: the menu applied the late-penalty percentage to the student's raw mark rather than to the assignment's maximum mark, and did not apply the zero-mark rule after more than three late days. The lecturer menu now calculates `max(0, rawMark - maxMark * penalty)`, with a 100% penalty after three days late. The existing system test covers both the one-day case and the more-than-three-days case.

Follow-up local verification (GNU C++ 14.2.0, CMake 3.31.6):

- CMake configure: passed.
- Full application and test build: passed.
- CTest: **126/126 passed**, including `ConsoleWorkflowSystemTest`.
- The test run includes startup failure handling, submission transaction rollback, one-day late penalty calculation, and resubmission history / zero-mark behaviour.

This is local Linux verification only


### Review of the latest supplied archive

- Application-only CMake configure: passed with tests disabled, vendored SQLiteCpp enabled, and vendored bcrypt enabled.
- Application build: passed using GNU C++ 14.2.0.
- Compiler warnings found and corrected: two multi-character newline literals in `AdminMenu.cpp` were changed to ordinary newline character literals; unused menu helper functions were removed.
- Rebuild after these corrections: passed without compiler warnings in the rebuilt menu translation units.
- Test configuration from the supplied ZIP: blocked because `third_party/SQLiteCpp/googletest` is an empty directory in the archive. The root `.gitmodules` file references the submodule, but the archive does not contain its checked-out contents. Initialize the submodule in the Git working copy before running tests (`git submodule update --init --recursive`).
- This archive review does not replace the earlier recorded 126/126 local test result; it means the test suite could not be independently rerun from this ZIP.
- The code metrics report was regenerated from this archive and is saved as `docs/quality/code-metrics-2026-11-02.txt`.
