#include "domain/late_penalty/TenPercentPerDayStrategy.h"

#include <algorithm>

namespace lms::domain {

double TenPercentPerDayStrategy::apply(double rawMark, double maximumMark,  int daysLate) const {
    if (daysLate <= 0) {
        return std::clamp(rawMark, 0.0, maximumMark);
    }
    if (daysLate > 3 || maximumMark <= 0.0) {
        return 0.0;
    }

    const double penalty = maximumMark * 0.10 * static_cast<double>(daysLate);
    return std::clamp(rawMark - penalty, 0.0, maximumMark);
}

} // namespace lms::domain
