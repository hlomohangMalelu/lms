#include "domain/department/Department.h"

#include <utility>

namespace lms::domain {

    Department::Department(
        long long departmentId,
        std::string name
    )
        : departmentId_(departmentId),
        name_(std::move(name)) {}

    long long Department::departmentId() const noexcept {
        return departmentId_;
    }

    const std::string& Department::name() const noexcept {
        return name_;
    }

}