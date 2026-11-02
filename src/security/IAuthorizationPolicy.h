#pragma once
namespace lms::security {
class IAuthorizationPolicy {
public:
    virtual ~IAuthorizationPolicy() = default;
    virtual void requireAdministrator(long long userId) const = 0;
    virtual void requireCanSubmit(long long studentId, long long assignmentId) const = 0;
    virtual void requireCanGrantExtension(long long lecturerId, long long assignmentId, long long studentId) const = 0;
    virtual void requireCanGrantClassExtension(long long lecturerId, long long assignmentId) const = 0;
    virtual void requireCanMark(long long lecturerId, long long submissionId) const = 0;
    virtual void requireCanModerate(long long hodId, long long submissionId) const = 0;
};
}
