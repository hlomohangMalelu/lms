#pragma once

#include "domain/late_penalty/ILatePenaltyStrategy.h"

namespace lms::domain {

class TenPercentPerDayStrategy final : public ILatePenaltyStrategy {
public:
    [[nodiscard]] double apply(double rawMark,double maximumMark,int daysLate) const override;
};

} // namespace lms::domain
