#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "domain/mark_record/MarkRecord.h"

using namespace lms::domain;

TEST(MarkRecordTest, StoresRecordAndSubmissionIdentifiers) {
    MarkRecord record(
        1, 10,
        std::nullopt, std::nullopt, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt
    );

    EXPECT_EQ(record.markRecordId(), 1);
    EXPECT_EQ(record.submissionId(), 10);
}

TEST(MarkRecordTest, NewRecordHasNoModerationOrApprovalDetails) {
    // These details remain absent until the relevant workflow steps occur.
    MarkRecord record(
        1, 10,
        std::nullopt, std::nullopt, std::nullopt,
        std::nullopt, std::nullopt, std::nullopt
    );

    EXPECT_FALSE(record.moderatedBy().has_value());
    EXPECT_FALSE(record.moderatedMark().has_value());
    EXPECT_FALSE(record.moderationNotes().has_value());
    EXPECT_FALSE(record.moderatedAt().has_value());
    EXPECT_FALSE(record.approvedBy().has_value());
    EXPECT_FALSE(record.approvedAt().has_value());
}

TEST(MarkRecordTest, StoresModerationDetails) {
    MarkRecord record(
        1, 10,
        3, 85.0,
        std::string("Mark reviewed and adjusted."),
        std::string("2026-11-01 09:30:00"),
        std::nullopt, std::nullopt
    );

    ASSERT_TRUE(record.moderatedBy().has_value());
    EXPECT_EQ(record.moderatedBy().value(), 3);

    ASSERT_TRUE(record.moderatedMark().has_value());
    EXPECT_DOUBLE_EQ(record.moderatedMark().value(), 85.0);

    ASSERT_TRUE(record.moderationNotes().has_value());
    EXPECT_EQ(
        record.moderationNotes().value(),
        "Mark reviewed and adjusted."
    );

    ASSERT_TRUE(record.moderatedAt().has_value());
    EXPECT_EQ(
        record.moderatedAt().value(),
        "2026-11-01 09:30:00"
    );
}

TEST(MarkRecordTest, StoresApprovalDetails) {
    MarkRecord record(
        1, 10,
        3, 85.0,
        std::string("Mark reviewed."),
        std::string("2026-11-01 09:30:00"),
        3, std::string("2026-11-01 10:00:00")
    );

    ASSERT_TRUE(record.approvedBy().has_value());
    EXPECT_EQ(record.approvedBy().value(), 3);

    ASSERT_TRUE(record.approvedAt().has_value());
    EXPECT_EQ(
        record.approvedAt().value(),
        "2026-11-01 10:00:00"
    );
}
