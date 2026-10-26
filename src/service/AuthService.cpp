#include "service/AuthService.h"

#include <stdexcept>

namespace lms::service {

    AuthService::AuthService(
        SQLite::Database& db,
        const lms::security::IPasswordHasher& hasher
    )
        : database_(db),
        hasher_(hasher) {
    }

    std::optional<AuthenticatedUser> AuthService::login(
        const std::string& username,
        const std::string& password
    ) const {
        if (username.empty() || password.empty()) {
            return std::nullopt;
        }

        SQLite::Statement query(
            database_,
            "SELECT id, username, full_name, role, password_hash "
            "FROM users "
            "WHERE username = ? COLLATE NOCASE AND active = 1"
        );

        query.bind(1, username);

        if (!query.executeStep()) {
            return std::nullopt;
        }

        const auto hash =
            query.getColumn("password_hash").getString();

        if (!hasher_.verify(password, hash)) {
            return std::nullopt;
        }

        return AuthenticatedUser{
            query.getColumn("id").getInt64(),
            query.getColumn("username").getString(),
            query.getColumn("full_name").getString(),
            query.getColumn("role").getString()
        };
    }

    void AuthService::changePassword(
        long long userId,
        const std::string& currentPassword,
        const std::string& newPassword
    ) {
        if (newPassword.size() < 12) {
            throw std::invalid_argument(
                "New password must contain at least 12 characters."
            );
        }

        SQLite::Statement query(
            database_,
            "SELECT password_hash "
            "FROM users "
            "WHERE id = ? AND active = 1"
        );

        query.bind(1, static_cast<std::int64_t>(userId));

        if (
            !query.executeStep() ||
            !hasher_.verify(
                currentPassword,
                query.getColumn(0).getString()
            )
        ) {
            throw std::runtime_error(
                "Current password is incorrect."
            );
        }

        const auto hash = hasher_.hash(newPassword);

        SQLite::Statement update(
            database_,
            "UPDATE users "
            "SET password_hash = ? "
            "WHERE id = ? AND active = 1"
        );

        update.bind(1, hash);
        update.bind(2, static_cast<std::int64_t>(userId));
        update.exec();
    }

} // namespace lms::service
