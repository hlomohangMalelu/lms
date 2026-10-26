#pragma once

#include "security/IPasswordHasher.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <optional>
#include <string>

namespace lms::service {

    struct AuthenticatedUser {
        long long id;
        std::string username;
        std::string fullName;
        std::string role;
    };

    class AuthService {
        public:
            AuthService(
                SQLite::Database& database,
                const lms::security::IPasswordHasher& hasher
            );

            [[nodiscard]] std::optional<AuthenticatedUser> login(
                const std::string& username,
                const std::string& password
            ) const;

            void changePassword(
                long long userId,
                const std::string& currentPassword,
                const std::string& newPassword
            );

        private:
            SQLite::Database& database_;
            const lms::security::IPasswordHasher& hasher_;
    };

} // namespace lms::service
