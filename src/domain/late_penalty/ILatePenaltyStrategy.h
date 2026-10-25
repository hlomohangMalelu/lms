#pragma once

namespace lms::domain {

    class ILatePenaltyStrategy {
        public:
            virtual ~ILatePenaltyStrategy() = default;

            // Calculates the final mark after applying the late-work policy.
            virtual double calculateFinalMark(
                double rawMark,
                double maxMark,
                int daysLate
            ) const = 0;
    };

}
