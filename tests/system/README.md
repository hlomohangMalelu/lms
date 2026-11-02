# System Tests

`ConsoleWorkflowTest.py` is a black-box system test that launches the real `lms` console executable with disposable SQLite databases. CMake registers it as `ConsoleWorkflowSystemTest`, so it runs as part of `ctest` and CI.

It verifies:

- Startup failure is reported with a non-zero exit.
- An active-session SQLite write failure is reported without false success or partial submission rows.
- A one-day-late mark deducts 10% of the assignment maximum mark.
- More-than-three-days-late work is accepted and receives a zero final mark.
- Resubmission history retains both attempts and the latest attempt is current.

Run it through CTest after building `lms`:

```bash
ctest --test-dir build-coverage -R ConsoleWorkflowSystemTest -V
```

All test data is temporary and removed after execution. The script uses only the Python standard library.
