#include <gtest/gtest.h>

#include "domain/submission/Submission.h"
#include "repository/submission/SQLiteSubmissionRepository.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <optional>
#include <string>

namespace {

    class SQLiteSubmissionRepositoryTest : public ::testing::Test {
    protected:
        SQLite::Database database{
            ":memory:",
            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
        };

        lms::repository::SQLiteSubmissionRepository repository{database};

        void SetUp() override {
            database.exec(
                "CREATE TABLE submissions ("
                "id INTEGER PRIMARY KEY, "
                "assignment_id INTEGER NOT NULL, "
                "student_id INTEGER NOT NULL, "
                "status TEXT NOT NULL, "
                "days_late INTEGER NOT NULL DEFAULT 0, "
                "penalty_fraction REAL NOT NULL DEFAULT 0, "
                "raw_mark REAL, "
                "final_mark REAL, "
                "feedback TEXT, "
                "released_at TEXT, "
                "released_by INTEGER"
                ")"
            );
        }
    };

    TEST_F(SQLiteSubmissionRepositoryTest, SavesAndRetrievesSubmission) {
        const lms::domain::Submission original(
            1, 10, 20, "SUBMITTED", 0, 0.0,
            std::nullopt, std::nullopt, std::nullopt,
            std::nullopt, std::nullopt
        );

        repository.save(original);

        const auto result = repository.findById(1);

        EXPECT_EQ(result.submissionId(), 1);
        EXPECT_EQ(result.assignmentId(), 10);
        EXPECT_EQ(result.studentId(), 20);
        EXPECT_EQ(result.status(), "SUBMITTED");
        EXPECT_EQ(result.daysLate(), 0);
        EXPECT_DOUBLE_EQ(result.penaltyFraction(), 0.0);
        EXPECT_FALSE(result.rawMark().has_value());
        EXPECT_FALSE(result.finalMark().has_value());
        EXPECT_FALSE(result.feedback().has_value());
    }

    TEST_F(SQLiteSubmissionRepositoryTest, SavesAndRetrievesMarkedSubmission) {
        const lms::domain::Submission original(
            2, 10, 21, "MARKED", 1, 0.1,
            75.0, 65.0, std::string("Good work"),
            std::nullopt, std::nullopt
        );

        repository.save(original);

        const auto result = repository.findById(2);

        ASSERT_TRUE(result.rawMark().has_value());
        EXPECT_DOUBLE_EQ(result.rawMark().value(), 75.0);

        ASSERT_TRUE(result.finalMark().has_value());
        EXPECT_DOUBLE_EQ(result.finalMark().value(), 65.0);

        ASSERT_TRUE(result.feedback().has_value());
        EXPECT_EQ(result.feedback().value(), "Good work");
    }

    TEST_F(SQLiteSubmissionRepositoryTest, UpdatesExistingSubmission) {
        const lms::domain::Submission original(
            3, 10, 22, "SUBMITTED", 0, 0.0,
            std::nullopt, std::nullopt, std::nullopt,
            std::nullopt, std::nullopt
        );

        repository.save(original);

        const lms::domain::Submission updated(
            3, 10, 22, "MARKED", 0, 0.0,
            80.0, 80.0, std::string("Well done"),
            std::nullopt, std::nullopt
        );

        repository.update(updated);

        const auto result = repository.findById(3);

        EXPECT_EQ(result.status(), "MARKED");
        ASSERT_TRUE(result.rawMark().has_value());
        EXPECT_DOUBLE_EQ(result.rawMark().value(), 80.0);
        ASSERT_TRUE(result.finalMark().has_value());
        EXPECT_DOUBLE_EQ(result.finalMark().value(), 80.0);
        ASSERT_TRUE(result.feedback().has_value());
        EXPECT_EQ(result.feedback().value(), "Well done");
    }

    TEST_F(SQLiteSubmissionRepositoryTest, ThrowsWhenSubmissionDoesNotExist) {
        EXPECT_THROW(
            repository.findById(999),
            std::runtime_error
        );
    }

    TEST_F(SQLiteSubmissionRepositoryTest, ThrowsWhenUpdatingMissingSubmission) {
        const lms::domain::Submission missing(
            999, 10, 22, "SUBMITTED", 0, 0.0,
            std::nullopt, std::nullopt, std::nullopt,
            std::nullopt, std::nullopt
        );

        EXPECT_THROW(
            repository.update(missing),
            std::runtime_error
        );
    }

}
