#pragma once

#include "security/IPasswordHasher.h"

namespace lms::security {

// Implementation will be connected to the selected bcrypt library.
class BcryptPasswordHasher final : public IPasswordHasher {
public:
    [[nodiscard]] std::string hash(std::string_view password) const override;
    [[nodiscard]] bool verify(std::string_view password,
                              std::string_view storedHash) const override;
};

} // namespace lms::security
