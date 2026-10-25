
#include "domain/admin/Admin.h"
#include "domain/permissions/Permission.h"

#include <utility>

namespace lms::domain {

    Admin::Admin(
        UserId userId,
        std::string username,
        std::string passwordHash,
        bool active,
        std::string lastName,
        std::string firstName
    ): User(
            userId,
            "ADMIN",
            std::move(username),
            std::move(passwordHash),
            active,
            std::move(lastName),
            std::move(firstName)
        ) {}

    bool Admin::isAllowed(const std::string& permission) const {
        if (!isActive()) {
            return false;
        }

        return permission == Permission::ManageUsers
            || permission == Permission::ManageDepartments
            || permission == Permission::ManageCourses
            || permission == Permission::AssignLecturersToCourses
            || permission == Permission::EnrolStudents;
    }

    std::string Admin::getRole() const {
        return "ADMIN";
    }

}
