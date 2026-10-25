#pragma once

#include "domain/user/User.h"

namespace lms::domain {

    class Lecturer : public User {
        public:
            Lecturer(
                UserId userId,
                std::string username,
                std::string passwordHash,
                bool active,
                std::string lastName,
                std::string firstName,
                std::string staffNumber
            );

            std::string staffNumber() const;

            bool isAllowed(const std::string& permission) const override;
            std::string getRole() const override;

        private:
            std::string staffNumber_;
    };

}
