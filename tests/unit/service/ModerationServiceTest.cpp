#include "service/ModerationService.h"
#include "unit/support/AllowAuthorizationPolicy.h"
#include <gtest/gtest.h>
#include <limits>
#include <vector>

namespace {
using lms::domain::Submission;
using lms::domain::MarkRecord;
using lms::domain::AuditLog;
class SubmissionRepo final : public lms::repository::ISubmissionRepository {
public:
    Submission value{7,3,9,"MARKED",1,0.1,80.0,72.0,"Feedback",{}, {}};
    int updates{0};
    void save(const Submission&) override {}
    Submission findById(long long) override { return value; }
    void update(const Submission& s) override { value=s; ++updates; }
};
class MarkRepo final : public lms::repository::IMarkRecordRepository {
public:
    MarkRecord value{51,7,22,75.0,"Reviewed","moderated",{}, {}};
    int saves{0};
    void save(const MarkRecord& r) override { value=r; ++saves; }
    MarkRecord findBySubmissionId(long long) override { return value; }
};
class AuditRepo final : public lms::repository::IAuditLogRepository {
public: std::vector<AuditLog> entries;
    void save(const AuditLog& a) override { entries.push_back(a); }
};
struct Fixture {
    SubmissionRepo submissions; MarkRepo marks; AuditRepo audits;
    test_support::AllowAuthorizationPolicy authorization;
    lms::service::ModerationService service{submissions,marks,audits,authorization};
};
}
TEST(ModerationServiceTest, ModeratesMarkAndCreatesAuditAndApprovalRecord) {
    Fixture f;
    f.service.moderateMark(7,30,52,61,75.0,100.0,"Reviewed carefully","Moderation reason","now");
    EXPECT_EQ(f.submissions.value.status(),"MODERATED");
    EXPECT_DOUBLE_EQ(*f.submissions.value.finalMark(),75.0);
    EXPECT_EQ(f.marks.saves,1);
    EXPECT_EQ(*f.marks.value.moderatedBy(),30);
    EXPECT_EQ(*f.marks.value.moderatedMark(),75.0);
    EXPECT_EQ(f.audits.entries.size(),1u);
}
TEST(ModerationServiceTest, DoesNotCreateAuditWhenMarkIsUnchanged) {
    Fixture f;
    f.service.moderateMark(7,30,52,61,72.0,100.0,"Reviewed","Reason","now");
    EXPECT_TRUE(f.audits.entries.empty());
    EXPECT_EQ(f.marks.saves,1);
}
TEST(ModerationServiceTest, AllowsRemoderationOfPreviouslyModeratedSubmission) {
    Fixture f;
    f.submissions.value = Submission(7,3,9,"MODERATED",1,0.1,80.0,75.0,"Feedback",{},{});
    f.service.moderateMark(7,30,52,61,78.0,100.0,"Second review","Reason","now");
    EXPECT_DOUBLE_EQ(*f.submissions.value.finalMark(),78.0);
}
TEST(ModerationServiceTest, RejectsInvalidMarkBoundsAndNonFiniteValues) {
    for(double mark : {-1.0,100.1,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) {
        Fixture f;
        EXPECT_THROW(f.service.moderateMark(7,30,52,61,mark,100,"Notes","Reason","now"),std::invalid_argument);
    }
    Fixture f;
    EXPECT_THROW(f.service.moderateMark(7,30,52,61,10,0,"Notes","Reason","now"),std::invalid_argument);
}
TEST(ModerationServiceTest, RejectsBlankReasonNotesAndTimestamp) {
    Fixture f;
    EXPECT_THROW(f.service.moderateMark(7,30,52,61,70,100,"Notes"," \t","now"),std::invalid_argument);
    EXPECT_THROW(f.service.moderateMark(7,30,52,61,70,100," \n","Reason","now"),std::invalid_argument);
    EXPECT_THROW(f.service.moderateMark(7,30,52,61,70,100,"Notes","Reason",""),std::invalid_argument);
}
TEST(ModerationServiceTest, RejectsSubmissionNotYetMarked) {
    Fixture f;
    f.submissions.value = Submission(7,3,9,"SUBMITTED",0,0.0,{},{},{},{},{});
    EXPECT_THROW(f.service.moderateMark(7,30,52,61,70,100,"Notes","Reason","now"),std::logic_error);
}
TEST(ModerationServiceTest, RejectsSubmissionWithoutFinalMark) {
    Fixture f;
    f.submissions.value = Submission(7,3,9,"MARKED",0,0.0,70.0,{},"Feedback",{},{});
    EXPECT_THROW(f.service.moderateMark(7,30,52,61,70,100,"Notes","Reason","now"),std::logic_error);
}
TEST(ModerationServiceTest, ApprovesModeratedMark) {
    Fixture f;
    f.submissions.value = Submission(7,3,9,"MODERATED",0,0.0,70.0,75.0,"Feedback",{},{});
    f.service.approveMark(7,31,"approved");
    EXPECT_EQ(f.marks.saves,1);
    EXPECT_EQ(*f.marks.value.approvedBy(),31);
    EXPECT_EQ(*f.marks.value.approvedAt(),"approved");
}
TEST(ModerationServiceTest, RejectsApprovalWithMissingTimestampOrWrongState) {
    Fixture f;
    EXPECT_THROW(f.service.approveMark(7,31,""),std::invalid_argument);
    f.submissions.value = Submission(7,3,9,"MARKED",0,0.0,70.0,75.0,"Feedback",{},{});
    EXPECT_THROW(f.service.approveMark(7,31,"now"),std::logic_error);
}
TEST(ModerationServiceTest, RejectsApprovalWithoutModerationDecision) {
    Fixture f;
    f.submissions.value = Submission(7,3,9,"MODERATED",0,0.0,70.0,75.0,"Feedback",{},{});
    f.marks.value = MarkRecord(51,7,{},75.0,"Notes","then",{},{});
    EXPECT_THROW(f.service.approveMark(7,31,"now"),std::logic_error);
}
TEST(ModerationServiceTest, ReleasesApprovedModeratedMark) {
    Fixture f;
    f.submissions.value = Submission(7,3,9,"MODERATED",0,0.0,70.0,75.0,"Feedback",{},{});
    f.marks.value = MarkRecord(51,7,30,75.0,"Notes","then",31,"approved");
    f.service.releaseMark(7,31,"released");
    EXPECT_EQ(f.submissions.value.status(),"RELEASED");
    EXPECT_EQ(*f.submissions.value.releasedAt(),"released");
    EXPECT_EQ(*f.submissions.value.releasedBy(),31);
}
TEST(ModerationServiceTest, RejectsReleaseWhenNotApprovedOrNotModerated) {
    Fixture f;
    EXPECT_THROW(f.service.releaseMark(7,31,""),std::invalid_argument);
    f.submissions.value = Submission(7,3,9,"MARKED",0,0.0,70.0,75.0,"Feedback",{},{});
    EXPECT_THROW(f.service.releaseMark(7,31,"now"),std::logic_error);
    f.submissions.value = Submission(7,3,9,"MODERATED",0,0.0,70.0,75.0,"Feedback",{},{});
    f.marks.value = MarkRecord(51,7,30,75.0,"Notes","then",{},{});
    EXPECT_THROW(f.service.releaseMark(7,31,"now"),std::logic_error);
}
TEST(ModerationServiceTest, RejectsReleaseWithoutFinalMark) {
    Fixture f;
    f.submissions.value = Submission(7,3,9,"MODERATED",0,0.0,70.0,{},"Feedback",{},{});
    f.marks.value = MarkRecord(51,7,30,75.0,"Notes","then",31,"approved");
    EXPECT_THROW(f.service.releaseMark(7,31,"now"),std::logic_error);
}
