# Service Unit Tests

Service tests exercise business workflows against isolated in-memory SQLite databases or test doubles. Current coverage includes authentication, submission/resubmission/extensions, and administration

Run all automated tests from the project root with:

```bash
cmake --build build-coverage --parallel 2
ctest --test-dir build-coverage --output-on-failure
```
