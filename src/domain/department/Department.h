#pragma once

#include <string>

namespace lms::domain {

    class Department {
        private:
            long long departmentId_;
            std::string name_;

        public:
            Department(
                long long departmentId,
                std::string name
            );

            long long departmentId() const noexcept;
            const std::string& name() const noexcept;
    };

}