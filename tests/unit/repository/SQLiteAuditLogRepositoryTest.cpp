#include <gtest/gtest.h>

#include "domain/audit_log/AuditLog.h"
#include "repository/audit_log/SQLiteAuditLogRepository.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <optional>
#include <stdexcept>
#include <string>

namespace {

    class SQLiteAuditLogRepositoryTest : public ::testing::Test {
        protected:
            SQLite::Database database{
                ":memory:",
                SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
            };

            lms::repository::SQLiteAuditLogRepository repository{database};

            void SetUp() override {
                database.exec(
                    "CREATE TABLE audit_log ("
                    "id INTEGER PRIMARY KEY, "
                    "submission_id INTEGER NOT NULL, "
                    "changed_by INTEGER NOT NULL, "
                    "changed_at TEXT NOT NULL, "
                    "old_value REAL, "
                    "new_value REAL NOT NULL, "
                    "reason TEXT NOT NULL"
                    ")"
                );
            }
    };

    TEST_F(SQLiteAuditLogRepositoryTest, SavesFirstMarkWithNullOldValue) {
        const lms::domain::AuditLog entry(
            1, 10, 5, "2026-10-25 10:00:00",
            std::nullopt, 75.0, "Initial mark"
        );

        repository.save(entry);

        SQLite::Statement query(
            database,
            "SELECT submission_id, changed_by, old_value, "
            "new_value, reason FROM audit_log WHERE id = 1"
        );

        ASSERT_TRUE(query.executeStep());

        EXPECT_EQ(query.getColumn(0).getInt64(), 10);
        EXPECT_EQ(query.getColumn(1).getInt64(), 5);
        EXPECT_TRUE(query.isColumnNull(2));
        EXPECT_DOUBLE_EQ(query.getColumn(3).getDouble(), 75.0);
        EXPECT_EQ(query.getColumn(4).getString(), "Initial mark");
    }

    TEST_F(SQLiteAuditLogRepositoryTest, SavesMarkChangeWithOldAndNewValues) {
        const lms::domain::AuditLog entry(
            2, 11, 6, "2026-10-25 11:00:00",
            60.0, 80.0, "Mark corrected after review"
        );

        repository.save(entry);

        SQLite::Statement query(
            database,
            "SELECT old_value, new_value, reason "
            "FROM audit_log WHERE id = 2"
        );

        ASSERT_TRUE(query.executeStep());

        ASSERT_FALSE(query.isColumnNull(0));
        EXPECT_DOUBLE_EQ(query.getColumn(0).getDouble(), 60.0);
        EXPECT_DOUBLE_EQ(query.getColumn(1).getDouble(), 80.0);
        EXPECT_EQ(
            query.getColumn(2).getString(),
            "Mark corrected after review"
        );
    }

}
