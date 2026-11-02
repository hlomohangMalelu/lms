#include "service/MarkingService.h"
#include "unit/support/AllowAuthorizationPolicy.h"
#include "domain/late_penalty/TenPercentPerDayStrategy.h"

#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
using lms::domain::Submission;
using lms::domain::AuditLog;

class SubmissionRepo final : public lms::repository::ISubmissionRepository {
public:
    Submission value{7, 3, 9, "SUBMITTED", 1, 0.0, std::nullopt, std::nullopt,
                     std::nullopt, std::nullopt, std::nullopt};
    int updates{0};
    void save(const Submission&) override {}
    Submission findById(long long) override { return value; }
    void update(const Submission& updated) override { value = updated; ++updates; }
};
class AuditRepo final : public lms::repository::IAuditLogRepository {
public:
    std::vector<AuditLog> entries;
    void save(const AuditLog& entry) override { entries.push_back(entry); }
};

struct Fixture {
    SubmissionRepo submissions;
    AuditRepo audits;
    lms::domain::TenPercentPerDayStrategy strategy;
    test_support::AllowAuthorizationPolicy authorization;
    lms::service::MarkingService service{submissions, audits, strategy, authorization};
};
}

TEST(MarkingServiceTest, AppliesLatePenaltyAndPersistsMarkAndAudit) {
    Fixture f;
    f.service.markSubmission(7, 22, 41, 80.0, 100.0, "Good work", "Initial marking", "2026-10-31T10:00:00");
    ASSERT_EQ(f.submissions.updates, 1);
    EXPECT_EQ(f.submissions.value.status(), "MARKED");
    ASSERT_TRUE(f.submissions.value.rawMark()); EXPECT_DOUBLE_EQ(*f.submissions.value.rawMark(), 80.0);
    ASSERT_TRUE(f.submissions.value.finalMark()); EXPECT_DOUBLE_EQ(*f.submissions.value.finalMark(), 70.0);
    ASSERT_EQ(f.audits.entries.size(), 1u);
    EXPECT_EQ(f.audits.entries[0].changedBy(), 22);
    EXPECT_DOUBLE_EQ(f.audits.entries[0].newValue(), 70.0);
}
TEST(MarkingServiceTest, AcceptsZeroAndMaximumMarks) {
    Fixture f;
    f.submissions.value = Submission(7, 3, 9, "SUBMITTED", 0, 0.0, {}, {}, {}, {}, {});
    EXPECT_NO_THROW(f.service.markSubmission(7, 22, 41, 0.0, 100.0, "", "Reason", "now"));
    EXPECT_DOUBLE_EQ(*f.submissions.value.finalMark(), 0.0);
    f.submissions.value = Submission(7, 3, 9, "SUBMITTED", 0, 0.0, {}, {}, {}, {}, {});
    EXPECT_NO_THROW(f.service.markSubmission(7, 22, 42, 100.0, 100.0, "", "Reason", "now"));
    EXPECT_DOUBLE_EQ(*f.submissions.value.finalMark(), 100.0);
}
TEST(MarkingServiceTest, RejectsNonPositiveOrNonFiniteMaximum) {
    for (double max : {0.0, -1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        Fixture f;
        EXPECT_THROW(f.service.markSubmission(7, 22, 41, 1.0, max, "", "Reason", "now"), std::invalid_argument);
    }
}
TEST(MarkingServiceTest, RejectsRawMarkOutsideRangeOrNonFinite) {
    for (double raw : {-0.01, 100.01, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        Fixture f;
        EXPECT_THROW(f.service.markSubmission(7, 22, 41, raw, 100.0, "", "Reason", "now"), std::invalid_argument);
    }
}
TEST(MarkingServiceTest, RejectsBlankReason) {
    Fixture f;
    EXPECT_THROW(f.service.markSubmission(7, 22, 41, 50.0, 100.0, "", " \t\n", "now"), std::invalid_argument);
}
TEST(MarkingServiceTest, RejectsMissingTimestamp) {
    Fixture f;
    EXPECT_THROW(f.service.markSubmission(7, 22, 41, 50.0, 100.0, "", "Reason", ""), std::invalid_argument);
}
TEST(MarkingServiceTest, CannotRemarkReleasedSubmission) {
    Fixture f;
    f.submissions.value = Submission(7, 3, 9, "RELEASED", 0, 0.0, 70.0, 70.0, "Feedback", "then", 4);
    EXPECT_THROW(f.service.markSubmission(7, 22, 41, 80.0, 100.0, "", "Reason", "now"), std::logic_error);
    EXPECT_TRUE(f.audits.entries.empty());
    EXPECT_EQ(f.submissions.updates, 0);
}
TEST(MarkingServiceTest, RecordsPreviousMarkInAuditEntry) {
    Fixture f;
    f.submissions.value = Submission(7, 3, 9, "MARKED", 0, 0.0, 60.0, 60.0, "Old", {}, {});
    f.service.markSubmission(7, 22, 41, 75.0, 100.0, "Updated", "Re-mark", "now");
    ASSERT_EQ(f.audits.entries.size(), 1u);
    ASSERT_TRUE(f.audits.entries[0].oldValue());
    EXPECT_DOUBLE_EQ(*f.audits.entries[0].oldValue(), 60.0);
}
TEST(MarkingServiceTest, SavesAuditBeforeUpdatingSubmission) {
    class OrderedAudit final : public lms::repository::IAuditLogRepository {
    public: explicit OrderedAudit(bool& updated) : updated_(updated) {}
        void save(const AuditLog&) override { EXPECT_FALSE(updated_); }
    private: bool& updated_;
    };
    class OrderedSubmission final : public lms::repository::ISubmissionRepository {
    public: bool updated{false};
        void save(const Submission&) override {}
        Submission findById(long long) override { return Submission(7,3,9,"SUBMITTED",0,0.0,{},{},{},{},{}); }
        void update(const Submission&) override { updated = true; }
    } repo;
    OrderedAudit audit(repo.updated);
    lms::domain::TenPercentPerDayStrategy strategy;
    test_support::AllowAuthorizationPolicy authorization;
    lms::service::MarkingService service(repo, audit, strategy, authorization);
    service.markSubmission(7,22,41,50,100,"Feedback","Reason","now");
    EXPECT_TRUE(repo.updated);
}
