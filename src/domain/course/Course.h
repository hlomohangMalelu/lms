
#pragma once

#include <string>

namespace lms::domain {

    class Course {
        private:
            long long courseId_;
            std::string code_;
            std::string title_;
            long long departmentId_;

        public:
            Course(
                long long courseId,
                std::string code,
                std::string title,
                long long departmentId
            );

            long long courseId() const noexcept;
            const std::string& code() const noexcept;
            const std::string& title() const noexcept;
            long long departmentId() const noexcept;
    };

}
