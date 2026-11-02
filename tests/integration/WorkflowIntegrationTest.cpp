#include <gtest/gtest.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include "domain/late_penalty/TenPercentPerDayStrategy.h"
#include "repository/audit_log/SQLiteAuditLogRepository.h"
#include "repository/mark_record/SQLiteMarkRecordRepository.h"
#include "repository/submission/SQLiteSubmissionRepository.h"
#include "security/BcryptPasswordHasher.h"
#include "security/SQLiteAuthorizationPolicy.h"
#include "service/AuthService.h"
#include "service/AdminService.h"
#include "service/MarkingService.h"
#include "service/ModerationService.h"
#include "service/SubmissionService.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string readSchema() {
    std::ifstream input(std::string(LMS_PROJECT_SOURCE_DIR) + "/db/schema.sql");
    if (!input) {
        throw std::runtime_error("Could not open db/schema.sql for integration test.");
    }
    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

class WorkflowIntegrationTest : public ::testing::Test {
protected:
    SQLite::Database db{":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE};
    lms::security::SQLiteAuthorizationPolicy authorization{db};
    lms::repository::SQLiteSubmissionRepository submissions{db};
    lms::repository::SQLiteAuditLogRepository audits{db};
    lms::repository::SQLiteMarkRecordRepository marks{db};
    lms::domain::TenPercentPerDayStrategy latePenalty;

    void SetUp() override {
        db.exec(readSchema());
        db.exec(R"SQL(
            INSERT INTO departments(id, name) VALUES (1, 'Computing'), (2, 'Business');
            INSERT INTO courses(id, code, title, department_id)
                VALUES (10, 'CS101', 'Software Engineering', 1),
                       (20, 'BUS101', 'Business Studies', 2);
            INSERT INTO users(id, username, password_hash, full_name, role, active) VALUES
                (1, 'student1', 'test-hash', 'Student One', 'STUDENT', 1),
                (2, 'student2', 'test-hash', 'Student Two', 'STUDENT', 1),
                (3, 'lecturer1', 'test-hash', 'Lecturer One', 'LECTURER', 1),
                (4, 'hod1', 'test-hash', 'Head One', 'HOD', 1),
                (5, 'admin1', 'test-hash', 'Admin One', 'ADMIN', 1);
            INSERT INTO students(user_id, student_no) VALUES (1, 'S001'), (2, 'S002');
            INSERT INTO lecturers(user_id, staff_no) VALUES (3, 'L001');
            INSERT INTO heads_of_department(user_id, department_id) VALUES (4, 1);
            INSERT INTO enrolments(student_id, course_id) VALUES (1, 10), (2, 20);
            INSERT INTO course_lecturers(lecturer_id, course_id) VALUES (3, 10);
            INSERT INTO assignments(id, course_id, created_by, title, instructions, deadline, max_mark)
                VALUES (100, 10, 3, 'Integration Assignment', 'Complete the work',
                        '2026-12-10 23:59:00', 100);
        )SQL");
    }

    long long scalar(const std::string& sql) {
        SQLite::Statement query(db, sql);
        if (!query.executeStep()) {
            throw std::runtime_error("Integration-test scalar query returned no row.");
        }
        return query.getColumn(0).getInt64();
    }
};

TEST_F(WorkflowIntegrationTest, AuthorizedSubmissionMarkModerationApprovalAndReleasePersistEndToEnd) {
    lms::service::SubmissionService submissionService(db, authorization);
    lms::service::MarkingService markingService(submissions, audits, latePenalty, authorization);
    lms::service::ModerationService moderationService(submissions, marks, audits, authorization);

    const long long submissionId = submissionService.submit(
        100, 1, "uploads/S001-assignment.pdf", "2026-12-09 12:00:00");

    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions WHERE id = 1 AND status = 'SUBMITTED'"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_files WHERE submission_id = 1"), 1);

    markingService.markSubmission(
        submissionId, 3, 1, 90.0, 100.0, "Good work", "Initial marking", "2026-12-10 10:00:00");
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions WHERE id = 1 AND status = 'MARKED' AND final_mark = 90"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM audit_log WHERE submission_id = 1"), 1);

    moderationService.moderateMark(
        submissionId, 4, 1, 2, 85.0, 100.0, "Adjusted against rubric",
        "Moderation adjustment", "2026-12-11 09:00:00");
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions WHERE id = 1 AND status = 'MODERATED' AND final_mark = 85"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM mark_records WHERE submission_id = 1 AND moderated_mark = 85"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM audit_log WHERE submission_id = 1"), 2);

    moderationService.approveMark(submissionId, 4, "2026-12-11 10:00:00");
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM mark_records WHERE submission_id = 1 AND approved_by = 4"), 1);

    moderationService.releaseMark(submissionId, 4, "2026-12-11 11:00:00");
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions WHERE id = 1 AND status = 'RELEASED' AND released_by = 4"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM audit_log WHERE submission_id = 1"), 2);
}

TEST_F(WorkflowIntegrationTest, ExtensionIsPersistedAndUsedWhenCalculatingSubmissionLateness) {
    lms::service::SubmissionService submissionService(db, authorization);

    submissionService.grantExtension(
        1, 100, 1, 3, "2026-12-12 23:59:00", "Approved accommodation");

    EXPECT_EQ(scalar("SELECT COUNT(*) FROM extensions WHERE assignment_id = 100 AND student_id = 1"), 1);

    const long long submissionId = submissionService.submit(
        100, 1, "uploads/S001-late.pdf", "2026-12-11 12:00:00");

    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions WHERE id = 1 AND status = 'SUBMITTED' AND days_late = 0"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_files WHERE submission_id = 1"), 1);
    EXPECT_EQ(submissionId, 1);
}

TEST_F(WorkflowIntegrationTest, FailedSubmissionFilePersistenceRollsBackSubmissionRow) {
    lms::service::SubmissionService submissionService(db, authorization);

    // The schema's datetime constraint rejects this timestamp after the submission
    // insert has been attempted. The service transaction must roll the insert back.
    EXPECT_THROW(
        submissionService.submit(100, 1, "uploads/bad.pdf", "not-a-date"),
        SQLite::Exception);

    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions"), 0);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_files"), 0);
}

TEST_F(WorkflowIntegrationTest, AdministratorSetupPersistsRoleProfilesAndCourseRelationships) {
    lms::service::AdminService admin(db, authorization, 5);

    const auto departmentId = admin.createDepartment("Science");
    const auto courseId = admin.createCourse("SCI101", "General Science", departmentId);
    const auto studentId = admin.createUser("student3", "test-hash", "Student Three", "STUDENT");
    const auto lecturerId = admin.createUser("lecturer2", "test-hash", "Lecturer Two", "LECTURER");
    admin.registerStudent(studentId, "S003");
    admin.registerLecturer(lecturerId, "L002");
    admin.enrolStudent(studentId, courseId);
    admin.assignLecturerToCourse(lecturerId, courseId);

    EXPECT_EQ(scalar("SELECT COUNT(*) FROM departments WHERE id = " + std::to_string(departmentId)), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM courses WHERE id = " + std::to_string(courseId)), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM enrolments WHERE student_id = " + std::to_string(studentId) +
                      " AND course_id = " + std::to_string(courseId)), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM course_lecturers WHERE lecturer_id = " + std::to_string(lecturerId) +
                      " AND course_id = " + std::to_string(courseId)), 1);
}

TEST_F(WorkflowIntegrationTest, DeadlineBoundaryIsInclusiveAndOneSecondAfterIsLate) {
    lms::service::SubmissionService submissionService(db, authorization);

    const auto beforeId = submissionService.submit(100, 1, "before.pdf", "2026-12-10 23:58:59");
    EXPECT_EQ(scalar("SELECT days_late FROM submissions WHERE id = " + std::to_string(beforeId)), 0);

    // A second enrolled student is required for an independent exact-boundary submission.
    db.exec("INSERT INTO users(id, username, password_hash, full_name, role, active) "
            "VALUES(6, 'student3', 'test-hash', 'Student Three', 'STUDENT', 1)");
    db.exec("INSERT INTO students(user_id, student_no) VALUES(6, 'S003')");
    db.exec("INSERT INTO enrolments(student_id, course_id) VALUES(6, 10)");
    const auto exactId = submissionService.submit(100, 6, "exact.pdf", "2026-12-10 23:59:00");
    EXPECT_EQ(scalar("SELECT days_late FROM submissions WHERE id = " + std::to_string(exactId)), 0);

    // One second after the deadline is late by one calendar-day penalty unit.
    db.exec("INSERT INTO users(id, username, password_hash, full_name, role, active) "
            "VALUES(7, 'student4', 'test-hash', 'Student Four', 'STUDENT', 1)");
    db.exec("INSERT INTO students(user_id, student_no) VALUES(7, 'S004')");
    db.exec("INSERT INTO enrolments(student_id, course_id) VALUES(7, 10)");
    const auto afterId = submissionService.submit(100, 7, "after.pdf", "2026-12-10 23:59:01");
    EXPECT_EQ(scalar("SELECT status = 'LATE' AND days_late = 1 FROM submissions WHERE id = " +
                     std::to_string(afterId)), 1);
}

TEST_F(WorkflowIntegrationTest, ResubmissionPreservesEarlierAttemptHistory) {
    lms::service::SubmissionService submissionService(db, authorization);

    const auto first = submissionService.submit(100, 1, "attempt-1.pdf", "2026-12-09 10:00:00");
    const auto second = submissionService.submit(100, 1, "attempt-2.pdf", "2026-12-09 11:00:00");

    EXPECT_EQ(first, second);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions WHERE id = 1"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_attempts WHERE submission_id = 1"), 2);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_attempts WHERE submission_id = 1 AND attempt_number = 1 AND file_path = 'attempt-1.pdf'"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_attempts WHERE submission_id = 1 AND attempt_number = 2 AND file_path = 'attempt-2.pdf'"), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_files WHERE submission_id = 1 AND file_path = 'attempt-2.pdf'"), 1);
}

TEST_F(WorkflowIntegrationTest, ExtensionDeadlineIsInclusiveAndOneSecondAfterIsLate) {
    lms::service::SubmissionService submissionService(db, authorization);
    submissionService.grantExtension(1, 100, 1, 3, "2026-12-12 23:59:00", "Approved extension");

    const auto exactId = submissionService.submit(100, 1, "extension-exact.pdf", "2026-12-12 23:59:00");
    EXPECT_EQ(scalar("SELECT days_late FROM submissions WHERE id = " + std::to_string(exactId)), 0);

    db.exec("INSERT INTO users(id, username, password_hash, full_name, role, active) "
            "VALUES(6, 'extensionstudent', 'test-hash', 'Extension Student', 'STUDENT', 1)");
    db.exec("INSERT INTO students(user_id, student_no) VALUES(6, 'S006')");
    db.exec("INSERT INTO enrolments(student_id, course_id) VALUES(6, 10)");
    submissionService.grantExtension(2, 100, 6, 3, "2026-12-12 23:59:00", "Approved extension");
    const auto afterId = submissionService.submit(100, 6, "extension-after.pdf", "2026-12-13 00:00:00");
    EXPECT_EQ(scalar("SELECT status = 'LATE' AND days_late = 1 FROM submissions WHERE id = " +
                     std::to_string(afterId)), 1);
}

TEST_F(WorkflowIntegrationTest, LoginMeetsTwoSecondTargetWithEightHundredStudentAccounts) {
    lms::security::BcryptPasswordHasher hasher;
    const std::string passwordHash = hasher.hash("PerformanceTest123!");
    SQLite::Statement insertUser(db,
        "INSERT INTO users(id, username, password_hash, full_name, role, active) "
        "VALUES(?, ?, ?, ?, 'STUDENT', 1)");
    SQLite::Statement insertStudent(db,
        "INSERT INTO students(user_id, student_no) VALUES(?, ?)");

    for (int i = 1; i <= 800; ++i) {
        const int id = 1000 + i;
        insertUser.bind(1, id);
        insertUser.bind(2, "performance_student_" + std::to_string(i));
        insertUser.bind(3, passwordHash);
        insertUser.bind(4, "Performance Student " + std::to_string(i));
        insertUser.exec();
        insertUser.reset();

        insertStudent.bind(1, id);
        insertStudent.bind(2, "PS" + std::to_string(i));
        insertStudent.exec();
        insertStudent.reset();
    }

    lms::service::AuthService auth(db, hasher);
    const auto start = std::chrono::steady_clock::now();
    const auto authenticated = auth.login("performance_student_800", "PerformanceTest123!");
    const auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_TRUE(authenticated.has_value());
    EXPECT_EQ(authenticated->role, "STUDENT");
    EXPECT_LT(std::chrono::duration<double>(elapsed).count(), 2.0)
        << "Login exceeded the 2-second SRS target with 800 student accounts.";
}

TEST_F(WorkflowIntegrationTest, FourDaysLateSubmissionIsStoredAndReceivesZeroAfterMarking) {
    lms::service::SubmissionService submissionService(db, authorization);
    lms::service::MarkingService markingService(submissions, audits, latePenalty, authorization);

    const auto submissionId = submissionService.submit(
        100, 1, "four-days-late.pdf", "2026-12-14 23:59:00");
    EXPECT_EQ(scalar("SELECT days_late FROM submissions WHERE id = " + std::to_string(submissionId)), 4);

    markingService.markSubmission(submissionId, 3, 3, 80.0, 100.0,
                                  "Late submission", "Late penalty", "2026-12-15 09:00:00");
    EXPECT_EQ(scalar("SELECT final_mark FROM submissions WHERE id = " + std::to_string(submissionId)), 0);
    EXPECT_EQ(scalar("SELECT penalty_fraction FROM submissions WHERE id = " + std::to_string(submissionId)), 1);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM audit_log WHERE submission_id = " + std::to_string(submissionId)), 1);
}

TEST_F(WorkflowIntegrationTest, ActiveSessionDatabaseWriteFailureRollsBackAllSubmissionRows) {
    lms::service::SubmissionService submissionService(db, authorization);
    db.exec(R"SQL(
        CREATE TRIGGER reject_submission_file BEFORE INSERT ON submission_files
        BEGIN
            SELECT RAISE(ABORT, 'simulated disk/write failure');
        END;
    )SQL");

    EXPECT_THROW(submissionService.submit(100, 1, "work.pdf", "2026-12-09 12:00:00"), SQLite::Exception);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions"), 0);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_files"), 0);
}

TEST_F(WorkflowIntegrationTest, UnauthorizedCrossCourseSubmissionLeavesDatabaseUnchanged) {
    lms::service::SubmissionService submissionService(db, authorization);

    EXPECT_THROW(
        submissionService.submit(100, 2, "uploads/S002.pdf", "2026-12-09 12:00:00"),
        std::runtime_error);

    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submissions"), 0);
    EXPECT_EQ(scalar("SELECT COUNT(*) FROM submission_files"), 0);
}

} // namespace
