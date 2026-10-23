#include "domain/User.h"

#include <utility>

namespace lms::domain {

User::User(UserId id, std::string username, std::string displayName)
    : id_(id),
      username_(std::move(username)),
      displayName_(std::move(displayName)) {}

UserId User::id() const noexcept { return id_; }

const std::string& User::username() const noexcept { return username_; }

const std::string& User::displayName() const noexcept { return displayName_; }

} // namespace lms::domain
