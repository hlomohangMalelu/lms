#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "domain/audit_log/AuditLog.h"

using namespace lms::domain;

TEST(AuditLogTest, StoresIdentifiers) {
    AuditLog log(
        1, 10, 3,
        "2026-10-25 09:30:00",
        std::nullopt, 85.0,
        "Initial mark recorded."
    );

    EXPECT_EQ(log.auditLogId(), 1);
    EXPECT_EQ(log.submissionId(), 10);
    EXPECT_EQ(log.changedBy(), 3);
}

TEST(AuditLogTest, StoresTimestampAndReason) {
    AuditLog log(
        1, 10, 3,
        "2026-10-25 09:30:00",
        std::nullopt, 85.0,
        "Initial mark recorded."
    );

    EXPECT_EQ(log.changedAt(), "2026-10-25 09:30:00");
    EXPECT_EQ(log.reason(), "Initial mark recorded.");
}

TEST(AuditLogTest, FirstMarkHasNoOldValue) {
    AuditLog log(
        1, 10, 3,
        "2026-10-25 09:30:00",
        std::nullopt, 85.0,
        "Initial mark recorded."
    );

    EXPECT_FALSE(log.oldValue().has_value());
    EXPECT_DOUBLE_EQ(log.newValue(), 85.0);
}

TEST(AuditLogTest, StoresPreviousAndUpdatedMarks) {
    AuditLog log(
        2, 10, 3,
        "2026-10-25 10:30:00",
        75.0, 85.0,
        "Mark corrected after review."
    );

    ASSERT_TRUE(log.oldValue().has_value());
    EXPECT_DOUBLE_EQ(log.oldValue().value(), 75.0);
    EXPECT_DOUBLE_EQ(log.newValue(), 85.0);
}
