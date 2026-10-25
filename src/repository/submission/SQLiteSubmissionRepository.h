#pragma once

#include "repository/submission/ISubmissionRepository.h"

#include <SQLiteCpp/Database.h>

namespace lms::repository {

    class SQLiteSubmissionRepository : public ISubmissionRepository {
        public:
            explicit SQLiteSubmissionRepository(SQLite::Database& database);

            void save(const lms::domain::Submission& submission) override;

            lms::domain::Submission findById(long long submissionId) override;

            void update(const lms::domain::Submission& submission) override;

        private:
            // The database connection is owned by the application, not this repository.
            SQLite::Database& database_;
    };

}
