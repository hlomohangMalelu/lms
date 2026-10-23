#pragma once

#include <string>

namespace lms::domain {

using UserId = long long;

class User {
public:
    User(UserId id, std::string username, std::string displayName);
    virtual ~User() = default;

    [[nodiscard]] UserId id() const noexcept;
    [[nodiscard]] const std::string& username() const noexcept;
    [[nodiscard]] const std::string& displayName() const noexcept;

private:
    UserId id_;
    std::string username_;
    std::string displayName_;
};

} // namespace lms::domain
