#pragma once

#include "repository/mark_record/IMarkRecordRepository.h"

#include <SQLiteCpp/Database.h>

namespace lms::repository {

    class SQLiteMarkRecordRepository : public IMarkRecordRepository {
        public:
            explicit SQLiteMarkRecordRepository(SQLite::Database& database);

            void save(const lms::domain::MarkRecord& record) override;

            lms::domain::MarkRecord findBySubmissionId(
                long long submissionId
            ) override;

        private:
            SQLite::Database& database_;
    };

}
