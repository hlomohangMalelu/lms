#include <gtest/gtest.h>

#include "service/AdminService.h"
#include "unit/support/AllowAuthorizationPolicy.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <stdexcept>
#include <string>

namespace {

class AdminServiceTest : public ::testing::Test {
protected:
    SQLite::Database database{":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE};
    test_support::AllowAuthorizationPolicy authorization;
    lms::service::AdminService admin{database, authorization, 1};

    void SetUp() override {
        database.exec("PRAGMA foreign_keys = ON");
        database.exec("CREATE TABLE departments(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE)");
        database.exec("CREATE TABLE courses(id INTEGER PRIMARY KEY, code TEXT NOT NULL UNIQUE, title TEXT NOT NULL, department_id INTEGER NOT NULL REFERENCES departments(id))");
        database.exec("CREATE TABLE users(id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE, password_hash TEXT NOT NULL, full_name TEXT NOT NULL, role TEXT NOT NULL CHECK(role IN ('STUDENT','LECTURER','HOD','ADMIN')), active INTEGER NOT NULL DEFAULT 1)");
        database.exec("CREATE TABLE students(user_id INTEGER PRIMARY KEY REFERENCES users(id), student_no TEXT NOT NULL UNIQUE)");
        database.exec("CREATE TABLE lecturers(user_id INTEGER PRIMARY KEY REFERENCES users(id), staff_no TEXT NOT NULL UNIQUE)");
        database.exec("CREATE TABLE heads_of_department(user_id INTEGER PRIMARY KEY REFERENCES users(id), department_id INTEGER NOT NULL UNIQUE REFERENCES departments(id))");
        database.exec("CREATE TABLE course_lecturers(lecturer_id INTEGER NOT NULL REFERENCES lecturers(user_id), course_id INTEGER NOT NULL REFERENCES courses(id), PRIMARY KEY(lecturer_id,course_id))");
        database.exec("CREATE TABLE enrolments(student_id INTEGER NOT NULL REFERENCES students(user_id), course_id INTEGER NOT NULL REFERENCES courses(id), PRIMARY KEY(student_id,course_id))");
    }

    long long count(const std::string& table) {
        SQLite::Statement query(database, "SELECT COUNT(*) FROM " + table);
        EXPECT_TRUE(query.executeStep());
        return query.getColumn(0).getInt64();
    }
};

TEST_F(AdminServiceTest, CreatesDepartmentAndRejectsBlankName) {
    EXPECT_THROW(admin.createDepartment(" \t "), std::invalid_argument);
    const auto id = admin.createDepartment("Computing");
    EXPECT_GT(id, 0);
    EXPECT_EQ(count("departments"), 1);
}

TEST_F(AdminServiceTest, CreatesCourseAndRejectsMissingFields) {
    const auto department = admin.createDepartment("Computing");
    EXPECT_THROW(admin.createCourse("", "Systems", department), std::invalid_argument);
    EXPECT_THROW(admin.createCourse("CS1", "", department), std::invalid_argument);
    EXPECT_GT(admin.createCourse("CS1", "Systems", department), 0);
}

TEST_F(AdminServiceTest, CreatesUsersForAllSupportedRoles) {
    for (const auto* role : {"STUDENT", "LECTURER", "HOD", "ADMIN"}) {
        const auto id = admin.createUser(std::string("user_") + role, "stored-hash", "Test User", role);
        EXPECT_GT(id, 0);
    }
    EXPECT_EQ(count("users"), 4);
}

TEST_F(AdminServiceTest, RejectsMissingUserFieldsAndUnsupportedRole) {
    EXPECT_THROW(admin.createUser("", "hash", "Name", "STUDENT"), std::invalid_argument);
    EXPECT_THROW(admin.createUser("user", "", "Name", "STUDENT"), std::invalid_argument);
    EXPECT_THROW(admin.createUser("user", "hash", "", "STUDENT"), std::invalid_argument);
    EXPECT_THROW(admin.createUser("user", "hash", "Name", "OBSERVER"), std::invalid_argument);
}

TEST_F(AdminServiceTest, DeactivatesExistingUserAndRejectsUnknownId) {
    const auto id = admin.createUser("learner", "hash", "Learner", "STUDENT");
    admin.deactivateUser(id);
    SQLite::Statement query(database, "SELECT active FROM users WHERE id=?");
    query.bind(1, static_cast<std::int64_t>(id));
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getInt(), 0);
    EXPECT_THROW(admin.deactivateUser(999), std::runtime_error);
}

TEST_F(AdminServiceTest, RegistersStudentOnlyForStudentRole) {
    const auto student = admin.createUser("student", "hash", "Student", "STUDENT");
    const auto lecturer = admin.createUser("lecturer", "hash", "Lecturer", "LECTURER");
    admin.registerStudent(student, "S001");
    EXPECT_EQ(count("students"), 1);
    EXPECT_THROW(admin.registerStudent(lecturer, "S002"), std::runtime_error);
    EXPECT_THROW(admin.registerStudent(999, "S003"), std::runtime_error);
}

TEST_F(AdminServiceTest, RegistersLecturerOnlyForLecturerRole) {
    const auto lecturer = admin.createUser("lecturer", "hash", "Lecturer", "LECTURER");
    const auto student = admin.createUser("student", "hash", "Student", "STUDENT");
    admin.registerLecturer(lecturer, "L001");
    EXPECT_EQ(count("lecturers"), 1);
    EXPECT_THROW(admin.registerLecturer(student, "L002"), std::runtime_error);
    EXPECT_THROW(admin.registerLecturer(999, "L003"), std::runtime_error);
}

TEST_F(AdminServiceTest, AssignsHeadOfDepartmentOnlyToHod) {
    const auto department = admin.createDepartment("Computing");
    const auto hod = admin.createUser("hod", "hash", "Head", "HOD");
    const auto student = admin.createUser("student", "hash", "Student", "STUDENT");
    admin.assignHeadOfDepartment(hod, department);
    EXPECT_EQ(count("heads_of_department"), 1);
    EXPECT_THROW(admin.assignHeadOfDepartment(student, department), std::runtime_error);
    EXPECT_THROW(admin.assignHeadOfDepartment(999, department), std::runtime_error);
}

TEST_F(AdminServiceTest, AssignsLecturerToCourseAndEnrolsStudent) {
    const auto department = admin.createDepartment("Computing");
    const auto course = admin.createCourse("CS1", "Systems", department);
    const auto lecturer = admin.createUser("lecturer", "hash", "Lecturer", "LECTURER");
    const auto student = admin.createUser("student", "hash", "Student", "STUDENT");
    admin.registerLecturer(lecturer, "L001");
    admin.registerStudent(student, "S001");
    admin.assignLecturerToCourse(lecturer, course);
    admin.enrolStudent(student, course);
    EXPECT_EQ(count("course_lecturers"), 1);
    EXPECT_EQ(count("enrolments"), 1);
}

TEST_F(AdminServiceTest, DatabaseConstraintsRejectDuplicateAssociations) {
    const auto department = admin.createDepartment("Computing");
    const auto course = admin.createCourse("CS1", "Systems", department);
    const auto lecturer = admin.createUser("lecturer", "hash", "Lecturer", "LECTURER");
    const auto student = admin.createUser("student", "hash", "Student", "STUDENT");
    admin.registerLecturer(lecturer, "L001");
    admin.registerStudent(student, "S001");
    admin.assignLecturerToCourse(lecturer, course);
    admin.enrolStudent(student, course);
    EXPECT_THROW(admin.assignLecturerToCourse(lecturer, course), SQLite::Exception);
    EXPECT_THROW(admin.enrolStudent(student, course), SQLite::Exception);
}

} // namespace
