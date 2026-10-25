#include "domain/student/Student.h"
#include "domain/permissions/Permission.h"

#include <utility>

namespace lms::domain {

Student::Student(
    UserId userId,
    std::string username,
    std::string passwordHash,
    bool active,
    std::string lastName,
    std::string firstName,
    std::string studentNumber
): User(
        userId,
        "STUDENT",
        std::move(username),
        std::move(passwordHash),
        active,
        std::move(lastName),
        std::move(firstName)
      ),
      studentNumber_(std::move(studentNumber)) {}

std::string Student::studentNumber() const {
    return studentNumber_;
}

bool Student::isAllowed(const std::string& permission) const {
    if (!isActive()) {
        return false;
    }

    return permission == Permission::ViewEnrolledCourses
        || permission == Permission::ViewAssignments
        || permission == Permission::SubmitAssignment
        || permission == Permission::ResubmitBeforeDeadline
        || permission == Permission::ViewReleasedMarks
        || permission == Permission::ViewFeedback;
}

std::string Student::getRole() const {
    return "STUDENT";
}

} // namespace lms::domain
