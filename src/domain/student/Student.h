#pragma once

#include "domain/user/User.h"

namespace lms::domain {

class Student : public User {
    public:
        Student(
            UserId userId,
            std::string username,
            std::string passwordHash,
            bool active,
            std::string lastName,
            std::string firstName,
            std::string studentNumber
        );

        std::string studentNumber() const;

        bool isAllowed(const std::string& permission) const override;

        std::string getRole() const override;

    private:
        std::string studentNumber_;
    };

} // namespace lms::domain
