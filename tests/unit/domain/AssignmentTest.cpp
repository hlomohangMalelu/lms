#include <gtest/gtest.h>

#include "domain/assignment/Assignment.h"

using namespace lms::domain;

TEST(AssignmentTest, StoresAssignmentId) {
    Assignment assignment(
        1, 10, 5, "Project",
        "Build an LMS",
        "2026-11-30 23:59:00",
        100.0
    );

    EXPECT_EQ(assignment.assignmentId(), 1);
}

TEST(AssignmentTest, StoresCourseId) {
    Assignment assignment(
        1, 10, 5, "Project",
        "Build an LMS",
        "2026-11-30 23:59:00",
        100.0
    );

    EXPECT_EQ(assignment.courseId(), 10);
}

TEST(AssignmentTest, StoresLecturerId) {
    Assignment assignment(
        1, 10, 5, "Project",
        "Build an LMS",
        "2026-11-30 23:59:00",
        100.0
    );

    EXPECT_EQ(assignment.createdBy(), 5);
}

TEST(AssignmentTest, StoresTitleAndInstructions) {
    Assignment assignment(
        1, 10, 5, "Project",
        "Build an LMS",
        "2026-11-30 23:59:00",
        100.0
    );

    EXPECT_EQ(assignment.title(), "Project");
    EXPECT_EQ(assignment.instructions(), "Build an LMS");
}

TEST(AssignmentTest, StoresDeadlineAndMaximumMark) {
    Assignment assignment(
        1, 10, 5, "Project",
        "Build an LMS",
        "2026-11-30 23:59:00",
        100.0
    );

    EXPECT_EQ(assignment.deadline(), "2026-11-30 23:59:00");
    EXPECT_DOUBLE_EQ(assignment.maxMark(), 100.0);
}
