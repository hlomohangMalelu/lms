#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

namespace lms::service {

    class SubmissionService {
        public:
            explicit SubmissionService(SQLite::Database& database);

            long long submit(
                long long assignmentId,
                long long studentId,
                const std::string& filePath,
                const std::string& submittedAt
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
    };

} // namespace lms::service
