#pragma once

#include <string>

namespace lms::domain::Permission {

    //Admin
    inline const std::string ManageUsers = "manage_users";
    inline const std::string ManageDepartments = "manage_departments";
    inline const std::string ManageCourses = "manage_courses";
    inline const std::string AssignLecturersToCourses = "assign_lecturers_to_courses";
    inline const std::string EnrolStudents = "enrol_students";

    //Lecturer
    inline const std::string ViewAssignedCourses = "view_assigned_courses";
    inline const std::string ViewAssignments = "view_assignments";
    inline const std::string CreateAssignment = "create_assignment";
    inline const std::string EditOwnAssignment = "edit_own_assignment";
    inline const std::string GrantExtension = "grant_extension";
    inline const std::string ViewSubmissions = "view_submissions";
    inline const std::string MarkSubmission = "mark_submission";
    inline const std::string WriteFeedback = "write_feedback";

    //Hod
    inline const std::string ViewDepartmentCourses = "view_department_courses";
    inline const std::string ViewDepartmentMarks = "view_department_marks";
    inline const std::string ModerateMarks = "moderate_marks";
    inline const std::string ApproveMarks = "approve_marks";
    inline const std::string ReleaseMarks = "release_marks";

    //Student
    inline const std::string ViewEnrolledCourses = "view_enrolled_courses";
    inline const std::string SubmitAssignment = "submit_assignment";
    inline const std::string ResubmitBeforeDeadline = "resubmit_before_deadline";
    inline const std::string ViewReleasedMarks = "view_released_marks";
    inline const std::string ViewFeedback = "view_feedback";

} // namespace lms::domain::Permission
