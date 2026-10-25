
#pragma once

#include <optional>
#include <string>

/*------------------------------------------------------------------
*raw_mark, finalMark, feedback, releasedAt and releasedBy can be NULL
*until a lecturer marks the submission and HOD releases Marks. 
*Therefore they are represented by std::optional<Type>
*When a value exists, the optional contains a value. Otherwise, it is empty
*/

namespace lms::domain {

    class Submission {
        private:
            long long submissionId_;
            long long assignmentId_;
            long long studentId_;
            std::string status_;
            int daysLate_;
            double penaltyFraction_;
            std::optional<double> rawMark_;
            std::optional<double> finalMark_;
            std::optional<std::string> feedback_;
            std::optional<std::string> releasedAt_;
            std::optional<long long> releasedBy_;

        public:
            Submission(
                long long submissionId,
                long long assignmentId,
                long long studentId,
                std::string status,
                int daysLate,
                double penaltyFraction,
                std::optional<double> rawMark,
                std::optional<double> finalMark,
                std::optional<std::string> feedback,
                std::optional<std::string> releasedAt,
                std::optional<long long> releasedBy
            );

            long long submissionId() const noexcept;
            long long assignmentId() const noexcept;
            long long studentId() const noexcept;
            const std::string& status() const noexcept;
            int daysLate() const noexcept;
            double penaltyFraction() const noexcept;

            const std::optional<double>& rawMark() const noexcept;
            const std::optional<double>& finalMark() const noexcept;
            const std::optional<std::string>& feedback() const noexcept;
            const std::optional<std::string>& releasedAt() const noexcept;
            const std::optional<long long>& releasedBy() const noexcept;
        };

}
