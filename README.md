# Moderated Learning Management System

A C++17 console LMS backed by SQLite/SQLiteCpp and bcrypt password hashing.

## Implemented workflows

- **Authentication:** active-user login, bcrypt password verification, and password changes that verify the current password and require at least 12 characters for the new password.
- **Administrator:** create departments and courses; create users for all supported roles; register student/lecturer profiles; assign heads of department; assign lecturers to courses; enrol students; deactivate users; list users/courses.
- **Lecturer:** view assigned courses; create assignments with instructions, deadlines and maximum marks; view course submissions and file references; mark submissions with feedback and audit reasons; grant/update student extensions.
- **Student:** view enrolled courses and assignments; submit/resubmit a file reference within the deadline or the configured three-day late window; view released marks and feedback.
- **Head of Department:** view marked submissions within the HOD's department; moderate marks and record notes; approve moderated marks; release approved marks to students.
- **Persistence/security:** parameterized SQL for user-entered values, bcrypt hashes, SQLite foreign keys, unique constraints, append-only audit-log triggers, and transactions around submission updates and console mark/moderation workflows.
- **Domain/service code:** domain models, repositories, marking/moderation services, and a late-penalty strategy are retained in the project.

## Build and run (Fedora/Linux)

Dependencies: a C++17 compiler, CMake, make, and SQLite development headers. SQLiteCpp and bcrypt are vendored. GoogleTest is fetched by CMake when tests are enabled, which requires network access unless already cached.

```bash
cmake -S . -B build -DLMS_BUILD_TESTS=OFF
cmake --build build -j$(nproc)
./build/lms
```

Run the executable from the project root so it can load `db/schema.sql`. The database is created at `data/lms.db`.

### First login

On a fresh database the app creates this development administrator:

- Username: `admin`
- Password: `ChangeMe123!`

Change this password immediately after login. These are development bootstrap credentials, not production credentials. User creation requires an initial password of at least 12 characters.

## Test suite

```bash
cmake -S . -B build-tests -DLMS_BUILD_TESTS=ON
cmake --build build-tests -j$(nproc)
ctest --test-dir build-tests --output-on-failure
```

The existing unit tests cover domain classes and selected repository behavior. The application was built successfully with tests disabled; the full test suite and a complete multi-role end-to-end acceptance run have not been verified as part of this package build.

## Project Structure

```text
lms/
├── .github/
│   └── workflows/                 # Continuous integration workflows
├── data/                           # Runtime application data
│   └── submissions/                # Assignment submission files
├── db/                             # Database schema and seed data
├── docs/                           # Project documentation and testing records
├── src/
│   ├── common/                     # Shared utilities and validation
│   ├── domain/                     # Core business entities and domain rules
│   │   ├── admin/                  # Administrator domain
│   │   ├── assignment/             # Assignment domain
│   │   ├── audit_log/              # Audit logging domain
│   │   ├── course/                 # Course domain
│   │   ├── department/             # Department domain
│   │   ├── extension/              # Assignment extension domain
│   │   ├── head_of_department/     # Head of Department domain
│   │   ├── late_penalty/           # Late submission penalty strategies
│   │   ├── lecturer/               # Lecturer domain
│   │   ├── mark_record/            # Mark records and history
│   │   ├── permissions/            # Role permissions and access rules
│   │   ├── student/                # Student domain
│   │   ├── submission/             # Submission domain
│   │   └── user/                   # User identity and account domain
│   ├── menus/                      # Role-specific console menus
│   │   ├── admin/                  # Administrator menu
│   │   ├── student/                # Student menu
│   │   ├── lecturer/               # Lecturer menu
│   │   └── head_of_department/     # Head of Department menu
│   ├── presentation/               # Console input and output
│   ├── repository/                 # Data access abstractions and implementations
│   │   ├── audit_log/              # Audit log persistence
│   │   ├── interfaces/             # Repository interfaces
│   │   ├── mark_record/            # Mark record persistence
│   │   ├── sqlite/                 # SQLite database infrastructure
│   │   └── submission/             # Submission persistence
│   ├── security/                   # Password hashing and security components
│   ├── service/                    # Application services and workflows
│   └── main.cpp                    # Application entry point
├── tests/                          # Automated tests
│   ├── fakes/                      # Test doubles and fake implementations
│   ├── integration/                # Integration tests
│   ├── system/                     # System-level tests
│   └── unit/                       # Unit tests
├── third_party/                    # Vendored external dependencies
│   ├── bcrypt/                     # Password hashing dependency
│   ├── sqlite3/                    # SQLite database engine
│   └── SQLiteCpp/                  # C++ SQLite wrapper
├── .clang-format                   # C++ formatting configuration
├── .clang-tidy                     # C++ static analysis configuration
├── .gitignore                      # Git ignore rules
├── CMakeLists.txt                  # Main CMake build configuration
├── CMakeLists.txt.backup           # Backup build configuration
└── README.md                       # Project overview and documentation
```

### Architecture Overview

- **Domain layer:** Defines the core business entities and rules.
- **Service layer:** Coordinates application operations and business workflows.
- **Repository layer:** Handles database access and persistence.
- **Menus and presentation:** Provide role-specific navigation and console interaction.
- **Security layer:** Provides password hashing functionality.
- **Database:** Stores application records using SQLite.
- **Tests:** Organize unit, integration, and system-level verification.
- **Third-party dependencies:** Contain the external libraries used by the project.
