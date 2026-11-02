#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include "security/IAuthorizationPolicy.h"

namespace lms::security {

// Database-backed, deny-by-default authorization checks for service operations.
class SQLiteAuthorizationPolicy : public IAuthorizationPolicy {
public:
    explicit SQLiteAuthorizationPolicy(SQLite::Database& database) : database_(database) {}

    bool isActiveUser(long long userId, const char* role) const;
    bool isAdministrator(long long userId) const;
    bool canSubmitAssignment(long long studentId, long long assignmentId) const;
    bool canGrantExtension(long long lecturerId, long long assignmentId, long long studentId) const;
    bool canMarkSubmission(long long lecturerId, long long submissionId) const;
    bool canModerateSubmission(long long hodId, long long submissionId) const;

    void requireActiveRole(long long userId, const char* role) const;
    void requireAdministrator(long long userId) const override;
    void requireCanSubmit(long long studentId, long long assignmentId) const override;
    void requireCanGrantExtension(long long lecturerId, long long assignmentId, long long studentId) const override;
    void requireCanMark(long long lecturerId, long long submissionId) const override;
    void requireCanModerate(long long hodId, long long submissionId) const override;

private:
    SQLite::Database& database_;
};

} // namespace lms::security
