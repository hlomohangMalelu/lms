#include "domain/course/Course.h"

#include <utility>

namespace lms::domain {

    Course::Course(
        long long courseId,
        std::string code,
        std::string title,
        long long departmentId

    ) : courseId_(courseId),
        code_(std::move(code)),
        title_(std::move(title)),
        departmentId_(departmentId) {}

    long long Course::courseId() const noexcept {
        return courseId_;
    }

    const std::string& Course::code() const noexcept {
        return code_;
    }

    const std::string& Course::title() const noexcept {
        return title_;
    }

    long long Course::departmentId() const noexcept {
        return departmentId_;
    }

}
