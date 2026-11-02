# Moderated LMS 1.0 — Installation Guide

## 1. Overview

Moderated LMS is a C++17 console application. It stores application records in a local SQLite database and uses bcrypt password hashing. It does not require a web server or a separate database server.

## 2. Requirements

- A C++17-compatible compiler (GCC, Clang, or MSVC).
- CMake 3.20 or newer.
- Git (when cloning the repository with submodules).
- A terminal / command prompt.

SQLiteCpp, SQLite and bcrypt are included in the repository. GoogleTest is required only when building the test suite; initialise its submodule before enabling tests.

## 3. Obtain the source

Clone the repository and initialise its submodules:

```sh
git clone --recurse-submodules <repository-url>
cd lms
# If already cloned without submodules:
git submodule update --init --recursive
```

For a source archive, ensure all vendored dependency files are included. A ZIP created by GitHub's source-archive feature may not include checked-out submodule contents.

## 4. Build on Linux or macOS

From the project root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DLMS_BUILD_TESTS=OFF \
  -DLMS_ENABLE_SQLITECPP=ON \
  -DLMS_ENABLE_BCRYPT=ON
cmake --build build --parallel 2
```

Run the application from the project root because it loads `db/schema.sql` using a relative path:

```sh
./build/lms
```

On macOS, use the same commands with Xcode Command Line Tools or another C++17 toolchain installed. On Windows, generate with CMake and run the resulting executable from the project root; the executable path depends on the selected generator/configuration.

## 5. Run tests

Initialise the GoogleTest submodule, then:

```sh
cmake -S . -B build-tests -DCMAKE_BUILD_TYPE=Debug \
  -DLMS_BUILD_TESTS=ON \
  -DLMS_ENABLE_SQLITECPP=ON \
  -DLMS_ENABLE_BCRYPT=ON
cmake --build build-tests --parallel 2
ctest --test-dir build-tests --output-on-failure
```

## 6. First run and administrator account

The application creates `data/lms.db` and its schema on startup. On a fresh database it bootstraps a development administrator:

- Username: `admin`
- Initial password: `ChangeMe123!`

Log in and change this password immediately. Do not reuse the bootstrap password or distribute a database containing it. The bootstrap credentials are for initial setup, not production deployment.

## 7. Data, backup and restore

The database is stored at `data/lms.db`. Back up the database while the application is stopped, and also back up any submission files stored under `data/`. Keep backups access-controlled because they contain user and academic records. To restore, stop the application, replace the database and associated submission data from a consistent backup, then restart.

## 8. Uninstallation

Stop the application and remove the executable/build directory. Remove `data/` only if the institution intentionally wants to permanently delete the local database and submission files. Preserve a verified backup first.

## 9. Deployment limitations

This release is a local, single-instance console application using a file-based SQLite database. It does not provide remote hosting, multi-site synchronization, a browser interface, or automated backup scheduling. Deploy it on a trusted machine with operating-system access controls and a documented backup process.
