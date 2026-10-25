#pragma once

#include <string>

namespace lms::domain {

using UserId = long long;

class User {
    private:
        UserId userId_;
        std::string role_;
        std::string username_;
        std::string passwordHash_;
        bool active_;
        std::string lastName_;
        std::string firstName_;

    public:
        User(
            UserId userId,
            std::string role,
            std::string username,
            std::string passwordHash,
            bool active,
            std::string lastName,
            std::string firstName
        );

        virtual ~User() = default;

        UserId userId() const noexcept;
        const std::string& username() const noexcept;
        const std::string& passwordHash() const noexcept;
        const std::string& lastName() const noexcept;
        const std::string& firstName() const noexcept;
        bool isActive() const noexcept;

        virtual bool isAllowed(const std::string& permission) const = 0;

        virtual std::string getRole() const = 0;

};

} // namespace lms::domain
