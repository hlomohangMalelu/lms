#pragma once

namespace lms::domain {

enum class SubmissionStatus {
    Submitted,
    Late,
    Marked,
    Moderated,
    Released
};

} // namespace lms::domain
