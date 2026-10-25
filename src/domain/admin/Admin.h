#pragma once

#include "domain/user/User.h"

namespace lms::domain {

    class Admin : public User {
    public:
        Admin(
            UserId userId,
            std::string username,
            std::string passwordHash,
            bool active,
            std::string lastName,
            std::string firstName
        );

        bool isAllowed(const std::string& permission) const override;
        std::string getRole() const override;
    };

}
