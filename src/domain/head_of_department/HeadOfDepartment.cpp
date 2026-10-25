#include "domain/head_of_department/HeadOfDepartment.h"
#include "domain/permissions/Permission.h"

namespace lms::domain {

    HeadOfDepartment::HeadOfDepartment(
        UserId userId,
        std::string username,
        std::string passwordHash,
        bool active,
        std::string lastName,
        std::string firstName,
        long long departmentId
    ): User(
            userId,
            "HOD",
            std::move(username),
            std::move(passwordHash),
            active,
            std::move(lastName),
            std::move(firstName)
        ),
        departmentId_(departmentId) {}

    long long HeadOfDepartment::departmentId() const {
        return departmentId_;
    }

    bool HeadOfDepartment::isAllowed(const std::string& permission) const {
        if (!isActive()) {
            return false;
        }

        return permission == Permission::ViewDepartmentCourses
            || permission == Permission::ViewDepartmentMarks
            || permission == Permission::ModerateMarks
            || permission == Permission::ApproveMarks
            || permission == Permission::ReleaseMarks;
    }

    std::string HeadOfDepartment::getRole() const {
        return "HOD";
    }

}
