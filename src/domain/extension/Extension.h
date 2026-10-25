#pragma once

#include <string>

namespace lms::domain {

    class Extension {
        private:
            long long extensionId_;
            long long assignmentId_;
            long long studentId_;
            long long grantedBy_;
            std::string newDeadline_;
            std::string reason_;

        public:
            Extension(
                long long extensionId,
                long long assignmentId,
                long long studentId,
                long long grantedBy,
                std::string newDeadline,
                std::string reason
            );

            long long extensionId() const noexcept;
            long long assignmentId() const noexcept;
            long long studentId() const noexcept;
            long long grantedBy() const noexcept;
            const std::string& newDeadline() const noexcept;
            const std::string& reason() const noexcept;
    };

}
