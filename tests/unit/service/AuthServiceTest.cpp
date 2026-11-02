#include <gtest/gtest.h>

#include "service/AuthService.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <string_view>

namespace {

class FakePasswordHasher final : public lms::security::IPasswordHasher {
public:
    std::string hash(std::string_view password) const override {
        return "hash:" + std::string(password);
    }

    bool verify(std::string_view password,
                std::string_view storedHash) const override {
        return hash(password) == storedHash;
    }
};

class AuthServiceTest : public ::testing::Test {
protected:
    SQLite::Database database{":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE};
    FakePasswordHasher hasher;
    lms::service::AuthService auth{database, hasher};

    void SetUp() override {
        database.exec(
            "CREATE TABLE users ("
            "id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE COLLATE NOCASE, "
            "password_hash TEXT NOT NULL, full_name TEXT NOT NULL, "
            "role TEXT NOT NULL, active INTEGER NOT NULL DEFAULT 1)"
        );
        database.exec(
            "INSERT INTO users(username,password_hash,full_name,role,active) "
            "VALUES('learner','hash:correct-password','Test Learner','STUDENT',1), "
            "('inactive','hash:correct-password','Inactive User','STUDENT',0)"
        );
    }
};

TEST_F(AuthServiceTest, LogsInWithValidCredentials) {
    const auto user = auth.login("learner", "correct-password");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->username, "learner");
    EXPECT_EQ(user->fullName, "Test Learner");
    EXPECT_EQ(user->role, "STUDENT");
    EXPECT_GT(user->id, 0);
}

TEST_F(AuthServiceTest, UsernameLookupIsCaseInsensitive) {
    const auto user = auth.login("LEARNER", "correct-password");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->username, "learner");
}

TEST_F(AuthServiceTest, RejectsEmptyCredentials) {
    EXPECT_FALSE(auth.login("", "correct-password").has_value());
    EXPECT_FALSE(auth.login("learner", "").has_value());
}

TEST_F(AuthServiceTest, RejectsUnknownUsername) {
    EXPECT_FALSE(auth.login("missing", "correct-password").has_value());
}

TEST_F(AuthServiceTest, RejectsIncorrectPassword) {
    EXPECT_FALSE(auth.login("learner", "incorrect-password").has_value());
}

TEST_F(AuthServiceTest, RejectsInactiveAccount) {
    EXPECT_FALSE(auth.login("inactive", "correct-password").has_value());
}

TEST_F(AuthServiceTest, RejectsNewPasswordShorterThanTwelveCharacters) {
    EXPECT_THROW(auth.changePassword(1, "correct-password", "short"), std::invalid_argument);
}

TEST_F(AuthServiceTest, RejectsIncorrectCurrentPassword) {
    EXPECT_THROW(auth.changePassword(1, "wrong-password", "a-new-password"), std::runtime_error);
}

TEST_F(AuthServiceTest, ChangesPasswordAndAllowsLoginWithNewPassword) {
    auth.changePassword(1, "correct-password", "a-new-password");
    EXPECT_FALSE(auth.login("learner", "correct-password").has_value());
    EXPECT_TRUE(auth.login("learner", "a-new-password").has_value());
}

TEST_F(AuthServiceTest, RejectsPasswordChangeForMissingOrInactiveUser) {
    EXPECT_THROW(auth.changePassword(999, "correct-password", "a-new-password"), std::runtime_error);
    EXPECT_THROW(auth.changePassword(2, "correct-password", "a-new-password"), std::runtime_error);
}

} // namespace
