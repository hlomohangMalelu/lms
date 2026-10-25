#pragma once

#include "domain/mark_record/MarkRecord.h"

namespace lms::repository {

    class IMarkRecordRepository {
    public:
        virtual ~IMarkRecordRepository() = default;

        virtual void save(
            const lms::domain::MarkRecord& record
        ) = 0;

        virtual lms::domain::MarkRecord findBySubmissionId(
            long long submissionId
        ) = 0;
    };

}
