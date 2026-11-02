#include <gtest/gtest.h>

#include "service/SubmissionService.h"
#include "unit/support/AllowAuthorizationPolicy.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <stdexcept>
#include <string>

namespace {

class SubmissionServiceTest : public ::testing::Test {
protected:
    SQLite::Database database{":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE};
    test_support::AllowAuthorizationPolicy authorization;
    lms::service::SubmissionService service{database, authorization};

    void SetUp() override {
        database.exec("CREATE TABLE assignments(id INTEGER PRIMARY KEY, deadline TEXT NOT NULL)");
        database.exec(
            "CREATE TABLE extensions(id INTEGER PRIMARY KEY, assignment_id INTEGER NOT NULL, "
            "student_id INTEGER NOT NULL, granted_by INTEGER NOT NULL, new_deadline TEXT NOT NULL, "
            "reason TEXT NOT NULL, UNIQUE(assignment_id,student_id))"
        );
        database.exec(
            "CREATE TABLE class_extensions(id INTEGER PRIMARY KEY, assignment_id INTEGER UNIQUE NOT NULL, "
            "granted_by INTEGER NOT NULL, new_deadline TEXT NOT NULL, reason TEXT NOT NULL)"
        );
        database.exec(
            "CREATE TABLE submissions(id INTEGER PRIMARY KEY, assignment_id INTEGER NOT NULL, "
            "student_id INTEGER NOT NULL, status TEXT NOT NULL, days_late INTEGER NOT NULL DEFAULT 0, "
            "penalty_fraction REAL NOT NULL DEFAULT 0, raw_mark REAL, final_mark REAL, feedback TEXT, "
            "released_at TEXT, released_by INTEGER, UNIQUE(assignment_id,student_id))"
        );
        database.exec(
            "CREATE TABLE submission_attempts(id INTEGER PRIMARY KEY, submission_id INTEGER NOT NULL, "
            "attempt_number INTEGER NOT NULL, file_path TEXT NOT NULL, submitted_at TEXT NOT NULL, "
            "status TEXT NOT NULL, days_late INTEGER NOT NULL, UNIQUE(submission_id,attempt_number))"
        );
        database.exec("INSERT INTO assignments(id,deadline) VALUES(10,'2026-10-20 23:59:00')");
    }

    long long count(const std::string& table) {
        SQLite::Statement query(database, "SELECT COUNT(*) FROM " + table);
        EXPECT_TRUE(query.executeStep());
        return query.getColumn(0).getInt64();
    }
};

TEST_F(SubmissionServiceTest, RejectsEmptyFilePathOrTimestamp) {
    EXPECT_THROW(service.submit(10, 20, "", "2026-10-20 12:00:00"), std::invalid_argument);
    EXPECT_THROW(service.submit(10, 20, "work.pdf", ""), std::invalid_argument);
}

TEST_F(SubmissionServiceTest, RejectsUnknownAssignment) {
    EXPECT_THROW(service.submit(999, 20, "work.pdf", "2026-10-20 12:00:00"), std::runtime_error);
}

TEST_F(SubmissionServiceTest, SavesOnTimeSubmissionAndFileReference) {
    const auto id = service.submit(10, 20, "work.pdf", "2026-10-20 12:00:00");
    EXPECT_GT(id, 0);
    EXPECT_EQ(count("submissions"), 1);
    EXPECT_EQ(count("submission_files"), 1);
    SQLite::Statement query(database, "SELECT status, days_late FROM submissions WHERE id = ?");
    query.bind(1, static_cast<std::int64_t>(id));
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getString(), "SUBMITTED");
    EXPECT_EQ(query.getColumn(1).getInt(), 0);
}

TEST_F(SubmissionServiceTest, DeadlineIsInclusiveAtTheExactTimestamp) {
    const auto before = service.submit(10, 20, "before.pdf", "2026-10-20 23:58:59");
    EXPECT_GT(before, 0);
    // A separate assignment avoids the unique (assignment, student) constraint.
    database.exec("INSERT INTO assignments(id,deadline) VALUES(11,'2026-10-20 23:59:00')");
    (void)service.submit(11, 20, "exact.pdf", "2026-10-20 23:59:00");
    database.exec("INSERT INTO assignments(id,deadline) VALUES(12,'2026-10-20 23:59:00')");
    const auto after = service.submit(12, 20, "after.pdf", "2026-10-20 23:59:01");

    SQLite::Statement query(database,
        "SELECT assignment_id, status, days_late FROM submissions ORDER BY assignment_id");
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(1).getString(), "SUBMITTED");
    EXPECT_EQ(query.getColumn(2).getInt(), 0);
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(1).getString(), "SUBMITTED");
    EXPECT_EQ(query.getColumn(2).getInt(), 0);
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(1).getString(), "LATE");
    EXPECT_EQ(query.getColumn(2).getInt(), 1);
    EXPECT_EQ(count("submissions"), 3);
    (void)after;
}

TEST_F(SubmissionServiceTest, CalculatesLateDaysAndAllowsThreeDaysLate) {
    const auto id = service.submit(10, 20, "work.pdf", "2026-10-23 23:59:00");
    SQLite::Statement query(database, "SELECT status, days_late FROM submissions WHERE id = ?");
    query.bind(1, static_cast<std::int64_t>(id));
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getString(), "LATE");
    EXPECT_EQ(query.getColumn(1).getInt(), 3);
}

TEST_F(SubmissionServiceTest, AcceptsSubmissionMoreThanThreeDaysLateForZeroMarking) {
    const auto id = service.submit(10, 20, "work.pdf", "2026-10-24 23:59:00");
    EXPECT_GT(id, 0);
    EXPECT_EQ(count("submissions"), 1);
    SQLite::Statement query(database, "SELECT status, days_late FROM submissions WHERE id = ?");
    query.bind(1, static_cast<std::int64_t>(id));
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getString(), "LATE");
    EXPECT_EQ(query.getColumn(1).getInt(), 4);
}

TEST_F(SubmissionServiceTest, AppliesStudentExtensionWhenCalculatingLateness) {
    database.exec(
        "INSERT INTO extensions(id,assignment_id,student_id,granted_by,new_deadline,reason) "
        "VALUES(1,10,20,30,'2026-10-25 23:59:00','Approved extension')"
    );
    const auto id = service.submit(10, 20, "work.pdf", "2026-10-24 12:00:00");
    SQLite::Statement query(database, "SELECT status, days_late FROM submissions WHERE id = ?");
    query.bind(1, static_cast<std::int64_t>(id));
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getString(), "SUBMITTED");
    EXPECT_EQ(query.getColumn(1).getInt(), 0);
}

TEST_F(SubmissionServiceTest, ResubmissionUpdatesExistingRecordAndFileReference) {
    const auto firstId = service.submit(10, 20, "first.pdf", "2026-10-20 12:00:00");
    const auto secondId = service.submit(10, 20, "replacement.pdf", "2026-10-21 12:00:00");
    EXPECT_EQ(firstId, secondId);
    EXPECT_EQ(count("submissions"), 1);
    EXPECT_EQ(count("submission_attempts"), 2);
    SQLite::Statement history(database,
        "SELECT attempt_number, file_path FROM submission_attempts "
        "WHERE submission_id = ? ORDER BY attempt_number");
    history.bind(1, static_cast<std::int64_t>(firstId));
    ASSERT_TRUE(history.executeStep());
    EXPECT_EQ(history.getColumn(0).getInt(), 1);
    EXPECT_EQ(history.getColumn(1).getString(), "first.pdf");
    ASSERT_TRUE(history.executeStep());
    EXPECT_EQ(history.getColumn(0).getInt(), 2);
    EXPECT_EQ(history.getColumn(1).getString(), "replacement.pdf");
    SQLite::Statement query(database, "SELECT file_path FROM submission_files WHERE submission_id = ?");
    query.bind(1, static_cast<std::int64_t>(firstId));
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getString(), "replacement.pdf");
}

TEST_F(SubmissionServiceTest, RejectsResubmissionAfterMarkingOrRelease) {
    const auto id = service.submit(10, 20, "first.pdf", "2026-10-20 12:00:00");
    SQLite::Statement update(database, "UPDATE submissions SET status = 'MARKED' WHERE id = ?");
    update.bind(1, static_cast<std::int64_t>(id));
    update.exec();
    EXPECT_THROW(service.submit(10, 20, "replacement.pdf", "2026-10-20 13:00:00"), std::logic_error);
}

TEST_F(SubmissionServiceTest, AppliesClassExtensionWhenNoIndividualExtensionExists) {
    service.grantClassExtension(1, 10, 30, "2026-10-25 23:59:00", "Whole class extension");
    const auto id = service.submit(10, 20, "class-work.pdf", "2026-10-24 12:00:00");
    SQLite::Statement query(database, "SELECT status, days_late FROM submissions WHERE id = ?");
    query.bind(1, static_cast<std::int64_t>(id));
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getString(), "SUBMITTED");
    EXPECT_EQ(query.getColumn(1).getInt(), 0);
}

TEST_F(SubmissionServiceTest, IndividualExtensionOverridesClassExtension) {
    service.grantClassExtension(1, 10, 30, "2026-10-22 23:59:00", "Whole class extension");
    service.grantExtension(2, 10, 20, 30, "2026-10-25 23:59:00", "Individual extension");
    const auto id = service.submit(10, 20, "individual-work.pdf", "2026-10-24 12:00:00");
    SQLite::Statement query(database, "SELECT status, days_late FROM submissions WHERE id = ?");
    query.bind(1, static_cast<std::int64_t>(id));
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getString(), "SUBMITTED");
    EXPECT_EQ(query.getColumn(1).getInt(), 0);
}

TEST_F(SubmissionServiceTest, RejectsClassExtensionWithMissingDeadlineOrBlankReason) {
    EXPECT_THROW(service.grantClassExtension(1, 10, 30, "", "valid reason"), std::invalid_argument);
    EXPECT_THROW(service.grantClassExtension(1, 10, 30, "2026-10-25 23:59:00", "  \t"), std::invalid_argument);
}

TEST_F(SubmissionServiceTest, GrantsAndUpdatesClassExtension) {
    service.grantClassExtension(1, 10, 30, "2026-10-25 23:59:00", "First reason");
    service.grantClassExtension(2, 10, 31, "2026-10-26 23:59:00", "Updated reason");
    EXPECT_EQ(count("class_extensions"), 1);
    SQLite::Statement query(database, "SELECT granted_by, new_deadline, reason FROM class_extensions WHERE assignment_id=10");
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getInt64(), 31);
    EXPECT_EQ(query.getColumn(1).getString(), "2026-10-26 23:59:00");
    EXPECT_EQ(query.getColumn(2).getString(), "Updated reason");
}

TEST_F(SubmissionServiceTest, RejectsExtensionWithMissingDeadlineOrBlankReason) {
    EXPECT_THROW(service.grantExtension(1, 10, 20, 30, "", "valid reason"), std::invalid_argument);
    EXPECT_THROW(service.grantExtension(1, 10, 20, 30, "2026-10-25 23:59:00", "  \t"), std::invalid_argument);
}

TEST_F(SubmissionServiceTest, GrantsAndUpdatesAnExtension) {
    service.grantExtension(1, 10, 20, 30, "2026-10-25 23:59:00", "First reason");
    service.grantExtension(2, 10, 20, 31, "2026-10-26 23:59:00", "Updated reason");
    EXPECT_EQ(count("extensions"), 1);
    SQLite::Statement query(database, "SELECT granted_by, new_deadline, reason FROM extensions WHERE assignment_id=10 AND student_id=20");
    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getInt64(), 31);
    EXPECT_EQ(query.getColumn(1).getString(), "2026-10-26 23:59:00");
    EXPECT_EQ(query.getColumn(2).getString(), "Updated reason");
}

} // namespace
