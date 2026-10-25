#include "domain/assignment/Assignment.h"

#include <utility>

namespace lms::domain {

    Assignment::Assignment(
        long long assignmentId,
        long long courseId,
        long long createdBy,
        std::string title,
        std::string instructions,
        std::string deadline,
        double maxMark
    ): assignmentId_(assignmentId),
        courseId_(courseId),
        createdBy_(createdBy),
        title_(std::move(title)),
        instructions_(std::move(instructions)),
        deadline_(std::move(deadline)),
        maxMark_(maxMark) {}

    long long Assignment::assignmentId() const noexcept {
        return assignmentId_;
    }

    long long Assignment::courseId() const noexcept {
        return courseId_;
    }

    long long Assignment::createdBy() const noexcept {
        return createdBy_;
    }

    const std::string& Assignment::title() const noexcept {
        return title_;
    }

    const std::string& Assignment::instructions() const noexcept {
        return instructions_;
    }

    const std::string& Assignment::deadline() const noexcept {
        return deadline_;
    }

    double Assignment::maxMark() const noexcept {
        return maxMark_;
    }

}
