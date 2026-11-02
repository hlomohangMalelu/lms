# Phase 5 Verification and Validation — Completion Checklist

This checklist follows the CS4433 individual assignment Phase 5 requirements.

| Assignment requirement | Evidence in this project | Status |
|---|---|---|
| Test plan maps unit, integration, system and acceptance levels to artefacts | `test-plan.md`, including V-Model mappings and entry/exit criteria | Complete |
| Equivalence partitioning and boundary value analysis | `test-plan.md`; deadline, extension, late-day, mark, role and persistence partitions | Complete for documented/tested boundaries |
| Automated unit tests using GoogleTest or Catch2 | `tests/unit/` | Complete |
| Automated integration tests | `tests/integration/WorkflowIntegrationTest.cpp` | Complete; 11 integration test cases |
| Automated tests run through CTest and CI configuration | `tests/CMakeLists.txt`, `.github/workflows/ci.yml` | Configured; local execution verified. Hosted GitHub Actions run must be confirmed after pushing the archive to the repository. |
| At least 70% business-logic line coverage and actual figure | `coverage-report.md`, `evidence/coverage-final-2026-11-02.txt` | Complete; 95.29% (587/616) |
| System tests derived from use cases, including exception flows | `system-test-execution.md`, `tests/system/ConsoleWorkflowTest.py`, `evidence/console-system-test-2026-11-02.txt` | Complete for listed scenarios |
| Acceptance tests for all four roles | `system-acceptance-cases.md`, `acceptance-signoff.md` | Demonstrations recorded; AT-01 partial; formal stakeholder sign-off pending |
| Defect log with severity, cause, fix and retest | `defect-log.md` | Complete; closed defects and open medium-severity gaps are distinguished |
| Traceability between requirements, use cases, design, database and tests | `../traceability.csv` | Complete for current identified requirements; partial requirements are labelled |
| Full test results and reproducible evidence | `test-results.md`, `integration-test-execution.md`, `system-test-execution.md`, `evidence/` | Complete for recorded build/test run |

## Outstanding items before claiming unconditional acceptance

1. Push the final source to the hosted repository and verify the GitHub Actions run; the local build is green but a hosted run is not evidenced here.
2. Obtain actual stakeholder/supervisor sign-off using `acceptance-signoff.md`.
3. Implement and test the administrator user-account update requirement (FUNC-01).
4. Route console marking/moderation through the Phase 3 service layer (ARCH-01).

These remaining items are not hidden: they are recorded as external evidence or medium-severity implementation/design gaps. Do not alter their status to PASS without the corresponding work and retest evidence.
