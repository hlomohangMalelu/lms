#include <gtest/gtest.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include "security/SQLiteAuthorizationPolicy.h"
#include "service/SubmissionService.h"
#include "service/AdminService.h"
#include "service/MarkingService.h"
#include "service/ModerationService.h"
#include "domain/late_penalty/TenPercentPerDayStrategy.h"
#include "repository/submission/ISubmissionRepository.h"
#include "repository/audit_log/IAuditLogRepository.h"
#include "repository/mark_record/IMarkRecordRepository.h"
#include "domain/submission/Submission.h"
#include "domain/audit_log/AuditLog.h"
#include "domain/mark_record/MarkRecord.h"
#include <stdexcept>
#include <string>

namespace {

class NoTouchSubmissionRepository final : public lms::repository::ISubmissionRepository {
public:
 int reads{0}; int writes{0};
 void save(const lms::domain::Submission&) override { ++writes; }
 lms::domain::Submission findById(long long) override { ++reads; throw std::runtime_error("repository must not be reached"); }
 void update(const lms::domain::Submission&) override { ++writes; }
};
class NoTouchAuditRepository final : public lms::repository::IAuditLogRepository {
public: int writes{0}; void save(const lms::domain::AuditLog&) override { ++writes; }
};
class NoTouchMarkRecordRepository final : public lms::repository::IMarkRecordRepository {
public:
 int reads{0}; int writes{0};
 void save(const lms::domain::MarkRecord&) override { ++writes; }
 lms::domain::MarkRecord findBySubmissionId(long long) override { ++reads; throw std::runtime_error("repository must not be reached"); }
};

class SecurityDatabaseTest : public ::testing::Test {
protected:
 SQLite::Database db{":memory:", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE};
 lms::security::SQLiteAuthorizationPolicy policy{db};
 void SetUp() override {
  db.exec("CREATE TABLE departments(id INTEGER PRIMARY KEY,name TEXT)");
  db.exec("CREATE TABLE courses(id INTEGER PRIMARY KEY,code TEXT,title TEXT,department_id INTEGER)");
  db.exec("CREATE TABLE users(id INTEGER PRIMARY KEY,username TEXT,password_hash TEXT,full_name TEXT,role TEXT,active INTEGER)");
  db.exec("CREATE TABLE students(user_id INTEGER PRIMARY KEY,student_no TEXT)");
  db.exec("CREATE TABLE lecturers(user_id INTEGER PRIMARY KEY,staff_no TEXT)");
  db.exec("CREATE TABLE heads_of_department(user_id INTEGER PRIMARY KEY,department_id INTEGER)");
  db.exec("CREATE TABLE enrolments(student_id INTEGER,course_id INTEGER,PRIMARY KEY(student_id,course_id))");
  db.exec("CREATE TABLE course_lecturers(lecturer_id INTEGER,course_id INTEGER,PRIMARY KEY(lecturer_id,course_id))");
  db.exec("CREATE TABLE assignments(id INTEGER PRIMARY KEY,course_id INTEGER,created_by INTEGER,title TEXT,instructions TEXT,deadline TEXT,max_mark REAL)");
  db.exec("CREATE TABLE submissions(id INTEGER PRIMARY KEY,assignment_id INTEGER,student_id INTEGER,status TEXT,days_late INTEGER DEFAULT 0,penalty_fraction REAL DEFAULT 0,raw_mark REAL,final_mark REAL,feedback TEXT,released_at TEXT,released_by INTEGER)");
  db.exec("CREATE TABLE submission_files(submission_id INTEGER PRIMARY KEY,file_path TEXT,submitted_at TEXT)");
  db.exec("CREATE TABLE extensions(id INTEGER PRIMARY KEY,assignment_id INTEGER,student_id INTEGER,granted_by INTEGER,new_deadline TEXT,reason TEXT)");
  db.exec("CREATE TABLE class_extensions(id INTEGER PRIMARY KEY,assignment_id INTEGER UNIQUE,granted_by INTEGER,new_deadline TEXT,reason TEXT)");
  db.exec("CREATE TABLE mark_records(id INTEGER PRIMARY KEY,submission_id INTEGER)");
  db.exec("CREATE TABLE audit_log(id INTEGER PRIMARY KEY,submission_id INTEGER,changed_by INTEGER,changed_at TEXT,old_value REAL,new_value REAL,reason TEXT)");
  db.exec("INSERT INTO departments VALUES(1,'Computing'),(2,'Business')");
  db.exec("INSERT INTO courses VALUES(10,'CS1','Systems',1),(20,'BUS1','Business',2)");
  db.exec("INSERT INTO users VALUES(1,'s1','h','Student','STUDENT',1),(2,'s2','h','Other Student','STUDENT',1),(3,'l1','h','Lecturer','LECTURER',1),(4,'l2','h','Other Lecturer','LECTURER',1),(5,'h1','h','HoD','HOD',1),(6,'h2','h','Other HoD','HOD',1),(7,'a1','h','Admin','ADMIN',1),(8,'a2','h','Inactive Admin','ADMIN',0),(9,'l9','h','Inactive Lecturer','LECTURER',0)");
  db.exec("INSERT INTO students VALUES(1,'S1'),(2,'S2')");
  db.exec("INSERT INTO lecturers VALUES(3,'L1'),(4,'L2'),(9,'L9')");
  db.exec("INSERT INTO heads_of_department VALUES(5,1),(6,2)");
  db.exec("INSERT INTO enrolments VALUES(1,10)");
  db.exec("INSERT INTO course_lecturers VALUES(3,10),(9,10)");
  db.exec("INSERT INTO assignments VALUES(100,10,3,'A','', '2026-12-01 23:59:00',100),(200,20,4,'B','', '2026-12-01 23:59:00',100)");
  db.exec("INSERT INTO submissions VALUES(500,100,1,'SUBMITTED',0,0,NULL,NULL,NULL,NULL,NULL)");
 }
 long long count(const char* t) { SQLite::Statement q(db,std::string("SELECT COUNT(*) FROM ")+t); EXPECT_TRUE(q.executeStep()); return q.getColumn(0).getInt64(); }
};

TEST_F(SecurityDatabaseTest, UnenrolledStudentCannotSubmitAndNoDatabaseChanges) {
 lms::service::SubmissionService service(db,policy);
 EXPECT_THROW(service.submit(200,1,"/tmp/a.pdf","2026-11-01 10:00:00"),std::runtime_error);
 EXPECT_THROW(service.submit(100,2,"/tmp/b.pdf","2026-11-01 10:00:00"),std::runtime_error);
 EXPECT_EQ(count("submissions"),1); EXPECT_EQ(count("submission_files"),0);
}
TEST_F(SecurityDatabaseTest, InactiveStudentCannotSubmitAndNoDatabaseChanges) {
 db.exec("INSERT INTO users VALUES(11,'s11','h','Inactive','STUDENT',0)"); db.exec("INSERT INTO students VALUES(11,'S11')"); db.exec("INSERT INTO enrolments VALUES(11,10)");
 lms::service::SubmissionService service(db,policy);
 EXPECT_THROW(service.submit(100,11,"/tmp/a.pdf","2026-11-01 10:00:00"),std::runtime_error);
 EXPECT_EQ(count("submissions"),1); EXPECT_EQ(count("submission_files"),0);
}
TEST_F(SecurityDatabaseTest, LecturerCannotMarkOutsideAssignedCourseOrWhenInactive) {
 EXPECT_THROW(policy.requireCanMark(4,500),std::runtime_error);
 EXPECT_THROW(policy.requireCanMark(9,500),std::runtime_error);
 EXPECT_THROW(policy.requireCanMark(1,500),std::runtime_error);
 EXPECT_EQ(count("submissions"),1);
}
TEST_F(SecurityDatabaseTest, UnassignedLecturerCannotGrantExtensionAndNoRowsChange) {
 lms::service::SubmissionService service(db,policy);
 EXPECT_THROW(service.grantExtension(1,100,1,4,"2026-12-05 23:59:00","Reason"),std::runtime_error);
 EXPECT_THROW(service.grantExtension(1,100,1,9,"2026-12-05 23:59:00","Reason"),std::runtime_error);
 EXPECT_EQ(count("extensions"),0);
}
TEST_F(SecurityDatabaseTest, OnlyAssignedActiveLecturerCanGrantClassExtension) {
 lms::service::SubmissionService service(db,policy);
 EXPECT_NO_THROW(service.grantClassExtension(1,100,3,"2026-12-05 23:59:00","Whole class"));
 EXPECT_THROW(service.grantClassExtension(2,100,4,"2026-12-05 23:59:00","Wrong course"),std::runtime_error);
 EXPECT_THROW(service.grantClassExtension(3,100,9,"2026-12-05 23:59:00","Inactive"),std::runtime_error);
 EXPECT_EQ(count("class_extensions"),1);
}
TEST_F(SecurityDatabaseTest, DeniedMarkingDoesNotReadOrWriteRepositories) {
 NoTouchSubmissionRepository submissions; NoTouchAuditRepository audits;
 lms::domain::TenPercentPerDayStrategy penalty;
 lms::service::MarkingService service(submissions,audits,penalty,policy);
 EXPECT_THROW(service.markSubmission(500,4,1,70,100,"Feedback","Reason","2026-11-01 10:00:00"),std::runtime_error);
 EXPECT_EQ(submissions.reads,0); EXPECT_EQ(submissions.writes,0); EXPECT_EQ(audits.writes,0);
 EXPECT_EQ(count("submissions"),1); EXPECT_EQ(count("audit_log"),0);
}
TEST_F(SecurityDatabaseTest, DeniedModerationDoesNotReadOrWriteRepositories) {
 NoTouchSubmissionRepository submissions; NoTouchAuditRepository audits; NoTouchMarkRecordRepository marks;
 lms::service::ModerationService service(submissions,marks,audits,policy);
 EXPECT_THROW(service.moderateMark(500,6,1,1,70,100,"Notes","Reason","2026-11-01 10:00:00"),std::runtime_error);
 EXPECT_THROW(service.approveMark(500,6,"2026-11-01 10:00:00"),std::runtime_error);
 EXPECT_THROW(service.releaseMark(500,6,"2026-11-01 10:00:00"),std::runtime_error);
 EXPECT_EQ(submissions.reads,0); EXPECT_EQ(submissions.writes,0); EXPECT_EQ(audits.writes,0);
 EXPECT_EQ(marks.reads,0); EXPECT_EQ(marks.writes,0);
 EXPECT_EQ(count("submissions"),1); EXPECT_EQ(count("audit_log"),0); EXPECT_EQ(count("mark_records"),0);
}

TEST_F(SecurityDatabaseTest, HoDCannotModerateOutsideDepartment) {
 EXPECT_THROW(policy.requireCanModerate(6,500),std::runtime_error);
 EXPECT_THROW(policy.requireCanModerate(1,500),std::runtime_error);
 EXPECT_EQ(count("submissions"),1);
}
TEST_F(SecurityDatabaseTest, InactiveAdminAndNonAdminCannotConstructAdminService) {
 EXPECT_THROW(lms::service::AdminService(db,policy,8),std::runtime_error);
 EXPECT_THROW(lms::service::AdminService(db,policy,1),std::runtime_error);
 EXPECT_EQ(count("departments"),2); EXPECT_EQ(count("courses"),2); EXPECT_EQ(count("users"),9);
}
TEST_F(SecurityDatabaseTest, AssignedActorsAndActiveAdminAreAllowed) {
 EXPECT_NO_THROW(policy.requireCanSubmit(1,100));
 EXPECT_NO_THROW(policy.requireCanMark(3,500));
 EXPECT_NO_THROW(policy.requireCanModerate(5,500));
 EXPECT_NO_THROW(policy.requireAdministrator(7));
}
}
