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
git clone --recurse-submodules https://github.com/hlomohangMalelu/lms.git
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



## 4.1 Run tests

Initialise the GoogleTest submodule, then:

```sh
cmake -S . -B build-tests -DCMAKE_BUILD_TYPE=Debug \
  -DLMS_BUILD_TESTS=ON \
  -DLMS_ENABLE_SQLITECPP=ON \
  -DLMS_ENABLE_BCRYPT=ON
cmake --build build-tests --parallel 2
ctest --test-dir build-tests --output-on-failure
```

## 5. Install on Windows

### 5.1 Requirements

Before building the application, install:

- **Git for Windows** – to clone the repository and initialise submodules.
- **CMake 3.20 or newer** – ensure CMake is added to the system PATH.
- **Visual Studio 2022 Build Tools** or Visual Studio 2022 – select the **Desktop development with C++** workload. Ensure the MSVC compiler and Windows SDK are included.

SQLiteCpp, SQLite and bcrypt are included in the repository, so they do not need to be installed separately.

### 5.2 Obtain the source code

Open PowerShell and run:

```powershell
git clone --recurse-submodules https://github.com/hlomohangMalelu/lms.git
cd lms
```

If the repository was cloned without submodules, initialise them:

```powershell
git submodule update --init --recursive
```

### 5.3 Configure and build

From the project root, run:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release `
  -DLMS_BUILD_TESTS=OFF `
  -DLMS_ENABLE_SQLITECPP=ON `
  -DLMS_ENABLE_BCRYPT=ON
```

Build the application:

```powershell
cmake --build build --config Release --parallel 2
```

### 5.4 Run the application

Run the executable from the project root so that the application can locate `db/schema.sql`:

```powershell
.\build\Release\lms.exe
```

If a single-configuration generator is used instead of Visual Studio, the executable may be located at:

```powershell
.\build\lms.exe
```

On first launch, the application creates its local SQLite database at `data/lms.db` and initialises the database schema.

Use the initial administrator credentials:

- Username: `admin`
- Password: `ChangeMe123!`

Change the initial password immediately after logging in.

### 5.5 Run tests (optional)

To build and run the test suite, ensure the GoogleTest submodule has been initialised:

```powershell
git submodule update --init --recursive
```

Configure and build:

```powershell
cmake -S . -B build-tests -DCMAKE_BUILD_TYPE=Debug `
  -DLMS_BUILD_TESTS=ON `
  -DLMS_ENABLE_SQLITECPP=ON `
  -DLMS_ENABLE_BCRYPT=ON

cmake --build build-tests --config Debug --parallel 2
```

Run the tests:

```powershell
ctest --test-dir build-tests -C Debug --output-on-failure
```

### 5.6 Windows data storage and backups

The database is stored in the project's `data` directory. Stop the application before backing up `data/lms.db`. Also back up the actual submission files referenced by the database, including files stored outside the project directory.

Restrict access to the database and backups because they contain student and academic records.


## 6. First run and administrator account

The application creates `data/lms.db` and its schema on startup. On a fresh database it bootstraps a development administrator:

- Username: `admin`
- Initial password: `ChangeMe123!`

Log in and change this password immediately. Do not reuse the bootstrap password or distribute a database containing it. The bootstrap credentials are for initial setup, not production deployment.

## 7. Data, backup and restore

The database is stored at `data/lms.db`. Back up the database while the application is stopped, and also back up the actual files referenced by the paths recorded in `submission_files` and `submission_attempts`. Those files may be outside the project directory; preserve their directory structure or document how to restore their paths. Keep backups access-controlled because they contain user and academic records. To restore, stop the application, replace the database and restore the referenced submission files to their expected paths from a consistent backup, then restart.

## 8. Uninstallation

Stop the application and remove the executable/build directory. Remove `data/` only if the institution intentionally wants to permanently delete the local database and submission files. Preserve a verified backup first.

## 9. Deployment limitations

This release is a local, single-instance console application using a file-based SQLite database. It does not provide remote hosting, multi-site synchronization, a browser interface, or automated backup scheduling. Deploy it on a trusted machine with operating-system access controls and a documented backup process.
