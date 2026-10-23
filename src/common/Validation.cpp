#include "common/Validation.h"

namespace lms::common {

bool isNonEmpty(const std::string& value) {
    return !value.empty();
}

} // namespace lms::common
