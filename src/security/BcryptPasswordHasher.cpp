#include "security/BcryptPasswordHasher.h"
#include <bcrypt.h>

#include <stdexcept>
#include <string>

namespace lms::security {

std::string BcryptPasswordHasher::hash(std::string_view password) const {
    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty");
    }

    return bcrypt::generateHash(std::string(password));
}

bool BcryptPasswordHasher::verify(
    std::string_view password,
    std::string_view storedHash
) const {
    if (password.empty() || storedHash.empty()) {
        return false;
    }

    return bcrypt::validatePassword(
        std::string(password),
        std::string(storedHash)
    );
}

}



 // namespace lms::security
