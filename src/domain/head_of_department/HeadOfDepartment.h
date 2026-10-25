#pragma once

#include "domain/user/User.h"

namespace lms::domain {

    class HeadOfDepartment : public User {
        public:
            HeadOfDepartment(
                UserId userId,
                std::string username,
                std::string passwordHash,
                bool active,
                std::string lastName,
                std::string firstName,
                long long departmentId
            );

            long long departmentId() const;

            bool isAllowed(const std::string& permission) const override;
            std::string getRole() const override;

        private:
            long long departmentId_;
    };

}
