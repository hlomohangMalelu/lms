#pragma once

namespace lms::domain {

class ILatePenaltyStrategy {
public:
    virtual ~ILatePenaltyStrategy() = default;

    // Returns the mark after applying the late penalty.
    // Policy details and boundary behavior will be implemented and tested next.
    [[nodiscard]] virtual double apply(double rawMark,
                                       double maximumMark,
                                       int daysLate) const = 0;
};

} // namespace lms::domain
