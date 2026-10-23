#include "security/BcryptPasswordHasher.h"

#include <stdexcept>

namespace lms::security {

std::string BcryptPasswordHasher::hash(std::string_view) const {
    throw std::logic_error("bcrypt dependency is not connected yet");
}

bool BcryptPasswordHasher::verify(std::string_view, std::string_view) const {
    throw std::logic_error("bcrypt dependency is not connected yet");
}

} // namespace lms::security
