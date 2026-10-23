#pragma once

#include <string>
#include <string_view>

namespace lms::security {

class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;

    [[nodiscard]] virtual std::string hash(std::string_view password) const = 0;
    [[nodiscard]] virtual bool verify(std::string_view password,
                                      std::string_view storedHash) const = 0;
};

} // namespace lms::security
