# Business-Logic Coverage Report

**Coverage target:** at least 70% line coverage of the business-logic layer.  
**Measured scope:** C++ source files under `src/domain/` and `src/service/`. Repository, presentation, common utilities, third-party code, headers and test code are excluded.  
**Toolchain:** GCC 14.2.0 and GCC `gcov` 14.2.0.  
**Build:** Debug, C++17, `LMS_BUILD_TESTS=ON`, `LMS_ENABLE_SQLITECPP=ON`, `LMS_ENABLE_BCRYPT=ON`, `LMS_ENABLE_COVERAGE=ON`.

## Result

**Status: TARGET ACHIEVED.**

**95.29% line coverage — 587 of 616 executable lines.** This fresh measurement was reproduced on 2 November 2026 after the clean instrumented build and all 126 automated tests passed. It exceeds the assignment's 70% target by **25.29 percentage points**.

| Layer | Covered lines | Executable lines | Coverage |
|---|---:|---:|---:|
| Domain | 224 | 251 | 89.24% |
| Service | 363 | 365 | 99.45% |
| **Combined business logic** | **587** | **616** | **95.29%** |

## Per-file results

| Source file | Covered | Executable | Coverage |
|---|---:|---:|---:|
| `domain/admin/Admin.cpp` | 13 | 17 | 76.47% |
| `domain/assignment/Assignment.cpp` | 22 | 22 | 100.00% |
| `domain/audit_log/AuditLog.cpp` | 22 | 22 | 100.00% |
| `domain/course/Course.cpp` | 13 | 13 | 100.00% |
| `domain/department/Department.cpp` | 7 | 7 | 100.00% |
| `domain/extension/Extension.cpp` | 19 | 19 | 100.00% |
| `domain/head_of_department/HeadOfDepartment.cpp` | 15 | 19 | 78.95% |
| `domain/late_penalty/TenPercentPerDayStrategy.cpp` | 11 | 11 | 100.00% |
| `domain/lecturer/Lecturer.cpp` | 19 | 22 | 86.36% |
| `domain/mark_record/MarkRecord.cpp` | 25 | 25 | 100.00% |
| `domain/student/Student.cpp` | 14 | 20 | 70.00% |
| `domain/submission/Submission.cpp` | 34 | 34 | 100.00% |
| `domain/user/User.cpp` | 10 | 20 | 50.00% |
| `service/AdminService.cpp` | 103 | 103 | 100.00% |
| `service/AuthService.cpp` | 41 | 41 | 100.00% |
| `service/MarkingService.cpp` | 33 | 33 | 100.00% |
| `service/ModerationService.cpp` | 82 | 82 | 100.00% |
| `service/SubmissionService.cpp` | 104 | 106 | 98.11% |

## Reproduction

Run these commands from the project root on a GCC-based environment:

```bash
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DLMS_BUILD_TESTS=ON \
  -DLMS_ENABLE_SQLITECPP=ON -DLMS_ENABLE_BCRYPT=ON \
  -DLMS_ENABLE_COVERAGE=ON
cmake --build build-coverage --parallel 2
ctest --test-dir build-coverage --output-on-failure
python3 docs/testing/measure-coverage.py
```

The included script invokes `gcov` on the instrumented domain and service translation units compiled into `lms_tests`. It counts executable C++ source lines (`-` lines are excluded) and counts lines marked `#####` or `=====` as not covered. It excludes headers and all source files outside the specified domain and service directories. `gcovr` and `lcov` are not required.

The fresh verified run on 2 November 2026 used the current GCC/CMake/SQLite toolchain in the execution environment; it included SEC-01 authorization tests, 11 SQLite workflow integration test cases and a black-box console system test (126/126 total CTest tests passed). Coverage results can vary if source code, tests, compiler or compiler flags change. Delete `build-coverage` and repeat the commands for a clean measurement.

## Interpretation and follow-up

The combined business-logic line-coverage target is met. The service layer is very highly covered; remaining uncovered lines are concentrated in domain classes, particularly `User.cpp`, `Student.cpp` and `HeadOfDepartment.cpp`. These are potential areas for additional tests, but the assignment's 70% aggregate target has already been achieved.

This report measures line coverage, not branch coverage. Passing automated tests does not replace the separately documented system and acceptance tests; those scenarios must be executed and recorded independently.
