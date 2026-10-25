#pragma once

#include "domain/late_penalty/ILatePenaltyStrategy.h"

namespace lms::domain {

    class TenPercentPerDayStrategy : public ILatePenaltyStrategy {
        public:
            double calculateFinalMark(
                double rawMark,
                double maxMark,
                int daysLate
            ) const override;
    };

}
//lms::domain
