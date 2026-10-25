
#pragma once

#include <string>

namespace lms::domain {

    class Assignment {
        private:
            long long assignmentId_;
            long long courseId_;
            long long createdBy_;
            std::string title_;
            std::string instructions_;
            std::string deadline_;
            double maxMark_;

        public:
            Assignment(
                long long assignmentId,
                long long courseId,
                long long createdBy,
                std::string title,
                std::string instructions,
                std::string deadline,
                double maxMark
            );

            long long assignmentId() const noexcept;
            long long courseId() const noexcept;
            long long createdBy() const noexcept;
            const std::string& title() const noexcept;
            const std::string& instructions() const noexcept;
            const std::string& deadline() const noexcept;
            double maxMark() const noexcept;
    };

}
