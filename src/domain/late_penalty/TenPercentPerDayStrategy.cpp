
#include "domain/late_penalty/TenPercentPerDayStrategy.h"

#include <algorithm>
#include <stdexcept>

namespace lms::domain {

    double TenPercentPerDayStrategy::calculateFinalMark(
        double rawMark,
        double maxMark,
        int daysLate
    ) const {
        if (maxMark <= 0.0) {
            throw std::invalid_argument("Maximum mark must be greater than zero.");
        }

        if (rawMark < 0.0 || rawMark > maxMark) {
            throw std::invalid_argument(
                "Raw mark must be between zero and the maximum mark."
            );
        }

        if (daysLate < 0) {
            throw std::invalid_argument("Days late cannot be negative.");
        }

        // Work submitted more than three days late receives zero.
        if (daysLate > 3) {
            return 0.0;
        }

        // Deduct 10% of the available mark for each day late.
        const double penalty = maxMark * 0.10 * daysLate;

        return std::max(0.0, rawMark - penalty);
    }

}
