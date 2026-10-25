#include <gtest/gtest.h>

#include "domain/course/Course.h"

using namespace lms::domain;

TEST(CourseTest, StoresCourseId) {
    Course course(1, "CS4433", "Software Engineering", 10);

    EXPECT_EQ(course.courseId(), 1);
}

TEST(CourseTest, StoresCourseCode) {
    Course course(1, "CS4433", "Software Engineering", 10);

    EXPECT_EQ(course.code(), "CS4433");
}

TEST(CourseTest, StoresCourseTitle) {
    Course course(1, "CS4433", "Software Engineering", 10);

    EXPECT_EQ(course.title(), "Software Engineering");
}

TEST(CourseTest, StoresDepartmentId) {
    Course course(1, "CS4433", "Software Engineering", 10);

    EXPECT_EQ(course.departmentId(), 10);
}
