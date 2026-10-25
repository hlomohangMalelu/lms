#pragma once

#include "domain/submission/Submission.h"

namespace lms::repository {

    class ISubmissionRepository {
        public:
            virtual ~ISubmissionRepository() = default;

            // Saves a new submission to persistent storage.
            virtual void save(const lms::domain::Submission& submission) = 0;

            // Retrieves a submission by its unique database ID.
            virtual lms::domain::Submission findById(long long submissionId) = 0;

            // Updates an existing submission.
            virtual void update(const lms::domain::Submission& submission) = 0;
    };

}
