#include <gtest/gtest.h>

#include "domain/student/Student.h"
#include "domain/permissions/Permission.h"
#include "domain/lecturer/Lecturer.h"
#include "domain/head_of_department/HeadOfDepartment.h"
#include "domain/admin/Admin.h"

using namespace lms::domain;

//student
TEST(StudentTest, ReturnsStudentRole) {
    Student student(
        1,
        "student1",
        "hashed_password",
        true,
        "Mokoena",
        "Lerato",
        "STU001"
    );

    EXPECT_EQ(student.getRole(), "STUDENT");
}

TEST(StudentTest, AllowsViewingEnrolledCourses) {
    Student student(
        1,
        "student1",
        "hashed_password",
        true,
        "Mokoena",
        "Lerato",
        "STU001"
    );

    EXPECT_TRUE(student.isAllowed(Permission::ViewEnrolledCourses));
}

TEST(StudentTest, InactiveStudentHasNoPermissions) {
    Student student(
        1,
        "student1",
        "hashed_password",
        false,
        "Mokoena",
        "Lerato",
        "STU001"
    );

    EXPECT_FALSE(student.isAllowed(Permission::ViewEnrolledCourses));
}

//lecturer

TEST(LecturerTest, ReturnsLecturerRole) {
    Lecturer lecturer(
        2,
        "lecturer1",
        "hashed_password",
        true,
        "Molefe",
        "Thabo",
        "STF001"
    );

    EXPECT_EQ(lecturer.getRole(), "LECTURER");
}

TEST(LecturerTest, AllowsMarkingSubmissions) {
    Lecturer lecturer(
        2,
        "lecturer1",
        "hashed_password",
        true,
        "Molefe",
        "Thabo",
        "STF001"
    );

    EXPECT_TRUE(lecturer.isAllowed(Permission::MarkSubmission));
}


//hod

TEST(HeadOfDepartmentTest, ReturnsCorrectRole) {
    HeadOfDepartment hod(
        3,
        "hod1",
        "hashed_password",
        true,
        "Mokoena",
        "Naledi",
        1
    );

    EXPECT_EQ(hod.getRole(), "HOD");
}

TEST(HeadOfDepartmentTest, AllowsModeratingMarks) {
    HeadOfDepartment hod(
        3,
        "hod1",
        "hashed_password",
        true,
        "Mokoena",
        "Naledi",
        1
    );

    EXPECT_TRUE(hod.isAllowed(Permission::ModerateMarks));
}

//admin

TEST(AdminTest, ReturnsCorrectRole) {
    Admin admin(
        4,
        "admin1",
        "hashed_password",
        true,
        "Molefe",
        "Palesa"
    );

    EXPECT_EQ(admin.getRole(), "ADMIN");
}

TEST(AdminTest, AllowsManagingUsers) {
    Admin admin(
        4,
        "admin1",
        "hashed_password",
        true,
        "Molefe",
        "Palesa"
    );

    EXPECT_TRUE(admin.isAllowed(Permission::ManageUsers));
}


//polymorphism test
TEST(UserPolymorphismTest, DifferentRolesUseSameUserInterface) {
    Student student(
        1, "student1", "hash", true,
        "Mokoena", "Lerato", "STU001"
    );

    Lecturer lecturer(
        2, "lecturer1", "hash", true,
        "Molefe", "Thabo", "STF001"
    );

    Admin admin(
        3, "admin1", "hash", true,
        "Molefe", "Palesa"
    );

    User* users[] = {
        &student,
        &lecturer,
        &admin
    };

    EXPECT_EQ(users[0]->getRole(), "STUDENT");
    EXPECT_EQ(users[1]->getRole(), "LECTURER");
    EXPECT_EQ(users[2]->getRole(), "ADMIN");
}