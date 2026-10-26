#include "service/AdminService.h"

#include <stdexcept>

namespace lms::service {

AdminService::AdminService(SQLite::Database& db)
    : database_(db) {
}

long long AdminService::createDepartment(
    const std::string& name
) {
    if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::invalid_argument(
            "Department name is required."
        );
    }

    SQLite::Statement query(
        database_,
        "INSERT INTO departments(name) VALUES(?)"
    );

    query.bind(1, name);
    query.exec();

    return database_.getLastInsertRowid();
}

long long AdminService::createCourse(
    const std::string& code,
    const std::string& title,
    long long departmentId
) {
    if (code.empty() || title.empty()) {
        throw std::invalid_argument(
            "Course code and title are required."
        );
    }

    SQLite::Statement query(
        database_,
        "INSERT INTO courses(code, title, department_id) "
        "VALUES(?, ?, ?)"
    );

    query.bind(1, code);
    query.bind(2, title);
    query.bind(3, static_cast<std::int64_t>(departmentId));
    query.exec();

    return database_.getLastInsertRowid();
}

long long AdminService::createUser(
    const std::string& username,
    const std::string& passwordHash,
    const std::string& fullName,
    const std::string& role
) {
    if (
        username.empty() ||
        passwordHash.empty() ||
        fullName.empty()
    ) {
        throw std::invalid_argument(
            "Username, password hash and full name are required."
        );
    }

    if (
        role != "STUDENT" &&
        role != "LECTURER" &&
        role != "HOD" &&
        role != "ADMIN"
    ) {
        throw std::invalid_argument("Unsupported user role.");
    }

    SQLite::Statement query(
        database_,
        "INSERT INTO users "
        "(username, password_hash, full_name, role) "
        "VALUES(?, ?, ?, ?)"
    );

    query.bind(1, username);
    query.bind(2, passwordHash);
    query.bind(3, fullName);
    query.bind(4, role);
    query.exec();

    return database_.getLastInsertRowid();
}

void AdminService::deactivateUser(long long id) {
    SQLite::Statement query(
        database_,
        "UPDATE users SET active = 0 WHERE id = ?"
    );

    query.bind(1, static_cast<std::int64_t>(id));
    query.exec();

    if (database_.getChanges() == 0) {
        throw std::runtime_error("User not found.");
    }
}

void AdminService::registerStudent(
    long long id,
    const std::string& no
) {
    SQLite::Statement query(
        database_,
        "INSERT INTO students(user_id, student_no) "
        "SELECT id, ? FROM users "
        "WHERE id = ? AND role = 'STUDENT'"
    );

    query.bind(1, no);
    query.bind(2, static_cast<std::int64_t>(id));
    query.exec();

    if (database_.getChanges() == 0) {
        throw std::runtime_error(
            "Student user not found or role mismatch."
        );
    }
}

void AdminService::registerLecturer(
    long long id,
    const std::string& no
) {
    SQLite::Statement query(
        database_,
        "INSERT INTO lecturers(user_id, staff_no) "
        "SELECT id, ? FROM users "
        "WHERE id = ? AND role = 'LECTURER'"
    );

    query.bind(1, no);
    query.bind(2, static_cast<std::int64_t>(id));
    query.exec();

    if (database_.getChanges() == 0) {
        throw std::runtime_error(
            "Lecturer user not found or role mismatch."
        );
    }
}

void AdminService::assignHeadOfDepartment(
    long long id,
    long long dept
) {
    SQLite::Statement query(
        database_,
        "INSERT INTO heads_of_department(user_id, department_id) "
        "SELECT id, ? FROM users "
        "WHERE id = ? AND role = 'HOD'"
    );

    query.bind(1, static_cast<std::int64_t>(dept));
    query.bind(2, static_cast<std::int64_t>(id));
    query.exec();

    if (database_.getChanges() == 0) {
        throw std::runtime_error(
            "HOD user not found or role mismatch."
        );
    }
}

void AdminService::assignLecturerToCourse(
    long long lecturer,
    long long course
) {
    SQLite::Statement query(
        database_,
        "INSERT INTO course_lecturers(lecturer_id, course_id) "
        "VALUES(?, ?)"
    );

    query.bind(1, static_cast<std::int64_t>(lecturer));
    query.bind(2, static_cast<std::int64_t>(course));
    query.exec();
}

void AdminService::enrolStudent(
    long long student,
    long long course
) {
    SQLite::Statement query(
        database_,
        "INSERT INTO enrolments(student_id, course_id) "
        "VALUES(?, ?)"
    );

    query.bind(1, static_cast<std::int64_t>(student));
    query.bind(2, static_cast<std::int64_t>(course));
    query.exec();
}

} // namespace lms::service
