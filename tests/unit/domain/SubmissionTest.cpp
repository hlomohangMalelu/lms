#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "domain/submission/Submission.h"

using namespace lms::domain;

TEST(SubmissionTest, StoresSubmissionIdentifiers) {
    Submission submission(
        1, 10, 20, "SUBMITTED",
        0, 0.0, std::nullopt, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt
    );

    EXPECT_EQ(submission.submissionId(), 1);
    EXPECT_EQ(submission.assignmentId(), 10);
    EXPECT_EQ(submission.studentId(), 20);
}

TEST(SubmissionTest, StoresStatusAndLatePenalty) {
    Submission submission(
        1, 10, 20, "LATE",
        2, 0.2, std::nullopt, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt
    );

    EXPECT_EQ(submission.status(), "LATE");
    EXPECT_EQ(submission.daysLate(), 2);
    EXPECT_DOUBLE_EQ(submission.penaltyFraction(), 0.2);
}

TEST(SubmissionTest, UnmarkedSubmissionHasNoMarks) {
    // Marks remain absent until the submission has been marked.
    Submission submission(
        1, 10, 20, "SUBMITTED",
        0, 0.0, std::nullopt, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt
    );

    EXPECT_FALSE(submission.rawMark().has_value());
    EXPECT_FALSE(submission.finalMark().has_value());
}

TEST(SubmissionTest, StoresMarksAndFeedback) {
    Submission submission(
        1, 10, 20, "MARKED",
        0, 0.0, 80.0, 80.0,
        std::string("Good work."),
        std::nullopt, std::nullopt
    );

    ASSERT_TRUE(submission.rawMark().has_value());
    EXPECT_DOUBLE_EQ(submission.rawMark().value(), 80.0);

    ASSERT_TRUE(submission.finalMark().has_value());
    EXPECT_DOUBLE_EQ(submission.finalMark().value(), 80.0);

    ASSERT_TRUE(submission.feedback().has_value());
    EXPECT_EQ(submission.feedback().value(), "Good work.");
}

TEST(SubmissionTest, StoresMarkReleaseInformation) {
    Submission submission(
        1, 10, 20, "RELEASED",
        0, 0.0, 90.0, 90.0,
        std::string("Excellent work."),
        std::string("2026-11-01 10:00:00"),
        5
    );

    ASSERT_TRUE(submission.releasedAt().has_value());
    EXPECT_EQ(
        submission.releasedAt().value(),
        "2026-11-01 10:00:00"
    );

    ASSERT_TRUE(submission.releasedBy().has_value());
    EXPECT_EQ(submission.releasedBy().value(), 5);
}
