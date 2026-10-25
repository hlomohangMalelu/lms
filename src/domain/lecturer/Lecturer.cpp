
#include "domain/lecturer/Lecturer.h"
#include "domain/permissions/Permission.h"

#include <utility>

namespace lms::domain {

    Lecturer::Lecturer(
        UserId userId,
        std::string username,
        std::string passwordHash,
        bool active,
        std::string lastName,
        std::string firstName,
        std::string staffNumber
    ): User(
            userId,
            "LECTURER",
            std::move(username),
            std::move(passwordHash),
            active,
            std::move(lastName),
            std::move(firstName)
        ),
        staffNumber_(std::move(staffNumber)) {}

    std::string Lecturer::staffNumber() const {
        return staffNumber_;
    }

    bool Lecturer::isAllowed(const std::string& permission) const {
        if (!isActive()) {
            return false;
        }

        return permission == Permission::ViewAssignedCourses
            || permission == Permission::ViewAssignments
            || permission == Permission::CreateAssignment
            || permission == Permission::EditOwnAssignment
            || permission == Permission::GrantExtension
            || permission == Permission::ViewSubmissions
            || permission == Permission::MarkSubmission
            || permission == Permission::WriteFeedback;
    }

    std::string Lecturer::getRole() const {
        return "LECTURER";
    }

}
