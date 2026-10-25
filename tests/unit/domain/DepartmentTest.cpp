#include <gtest/gtest.h>

#include "domain/department/Department.h"

using namespace lms::domain;

TEST(DepartmentTest, StoresDepartmentId) {
    Department department(1, "Computer Science");

    EXPECT_EQ(department.departmentId(), 1);
}

TEST(DepartmentTest, StoresDepartmentName) {
    Department department(1, "Computer Science");

    EXPECT_EQ(department.name(), "Computer Science");
}

TEST(DepartmentTest, StoresDifferentDepartmentsIndependently) {
    Department computerScience(1, "Computer Science");
    Department mathematics(2, "Mathematics");

    EXPECT_NE(computerScience.departmentId(), mathematics.departmentId());
    EXPECT_NE(computerScience.name(), mathematics.name());
}
