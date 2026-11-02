#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include "security/IAuthorizationPolicy.h"

namespace lms::service {

    class SubmissionService {
        public:
            SubmissionService(SQLite::Database& database, const lms::security::IAuthorizationPolicy& authorization);

            long long submit(
                long long assignmentId,
                long long studentId,
                const std::string& filePath,
                const std::string& submittedAt
            );

            void grantClassExtension(
                long long extensionId,
                long long assignmentId,
                long long lecturerId,
                const std::string& newDeadline,
                const std::string& reason
            );

            void grantExtension(
                long long extensionId,
                long long assignmentId,
                long long studentId,
                long long lecturerId,
                const std::string& newDeadline,
                const std::string& reason
            );

        private:
            SQLite::Database& database_;
            const lms::security::IAuthorizationPolicy& authorization_;
    };

} // namespace lms::service
