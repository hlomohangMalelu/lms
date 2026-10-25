#include "domain/extension/Extension.h"

#include <utility>

namespace lms::domain {

    Extension::Extension(
        long long extensionId,
        long long assignmentId,
        long long studentId,
        long long grantedBy,
        std::string newDeadline,
        std::string reason

    ): extensionId_(extensionId),
        assignmentId_(assignmentId),
        studentId_(studentId),
        grantedBy_(grantedBy),
        newDeadline_(std::move(newDeadline)),
        reason_(std::move(reason)) {}

    long long Extension::extensionId() const noexcept {
        return extensionId_;
    }

    long long Extension::assignmentId() const noexcept {
        return assignmentId_;
    }

    long long Extension::studentId() const noexcept {
        return studentId_;
    }

    long long Extension::grantedBy() const noexcept {
        return grantedBy_;
    }

    const std::string& Extension::newDeadline() const noexcept {
        return newDeadline_;
    }

    const std::string& Extension::reason() const noexcept {
        return reason_;
    }

}
