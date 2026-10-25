#include <gtest/gtest.h>

#include <stdexcept>

#include "domain/late_penalty/TenPercentPerDayStrategy.h"

using namespace lms::domain;

TEST(TenPercentPerDayStrategyTest, DoesNotPenaliseOnTimeSubmission) {
    TenPercentPerDayStrategy strategy;

    EXPECT_DOUBLE_EQ(
        strategy.calculateFinalMark(80.0, 100.0, 0),
        80.0
    );
}

TEST(TenPercentPerDayStrategyTest, DeductsTenPercentPerLateDay) {
    TenPercentPerDayStrategy strategy;

    EXPECT_DOUBLE_EQ(
        strategy.calculateFinalMark(80.0, 100.0, 1),
        70.0
    );

    EXPECT_DOUBLE_EQ(
        strategy.calculateFinalMark(80.0, 100.0, 2),
        60.0
    );

    EXPECT_DOUBLE_EQ(
        strategy.calculateFinalMark(80.0, 100.0, 3),
        50.0
    );
}

TEST(TenPercentPerDayStrategyTest, GivesZeroAfterThreeDaysLate) {
    TenPercentPerDayStrategy strategy;

    EXPECT_DOUBLE_EQ(
        strategy.calculateFinalMark(80.0, 100.0, 4),
        0.0
    );
}

TEST(TenPercentPerDayStrategyTest, DoesNotReturnNegativeMark) {
    TenPercentPerDayStrategy strategy;

    EXPECT_DOUBLE_EQ(
        strategy.calculateFinalMark(5.0, 100.0, 1),
        0.0
    );
}

TEST(TenPercentPerDayStrategyTest, RejectsInvalidMaximumMark) {
    TenPercentPerDayStrategy strategy;

    EXPECT_THROW(
        strategy.calculateFinalMark(5.0, 0.0, 1),
        std::invalid_argument
    );
}

TEST(TenPercentPerDayStrategyTest, RejectsRawMarkAboveMaximum) {
    TenPercentPerDayStrategy strategy;

    EXPECT_THROW(
        strategy.calculateFinalMark(110.0, 100.0, 1),
        std::invalid_argument
    );
}

TEST(TenPercentPerDayStrategyTest, RejectsNegativeDaysLate) {
    TenPercentPerDayStrategy strategy;

    EXPECT_THROW(
        strategy.calculateFinalMark(80.0, 100.0, -1),
        std::invalid_argument
    );
}
