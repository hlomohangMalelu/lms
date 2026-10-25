
#include "domain/User.h"

#include <utility>

namespace lms::domain {

User::User(UserId userId, std::string role, std::string username, std::string passwordHash, bool active, std::string lastName, std::string firstName
) : userId_(userId),
    role_(std::move(role)),
    username_(std::move(username)),
    passwordHash_(std::move(passwordHash)),
    active_(active),
    lastName_(std::move(lastName)),
    firstName_(std::move(firstName)) {}

UserId User::userId() const noexcept {
  return userId_;
}

const std::string& User::username() const noexcept {
  return username_;
}

const std::string& User::passwordHash() const noexcept {
  return passwordHash_;
}

const std::string& User::lastName() const noexcept {
  return lastName_;
}

const std::string& User::firstName() const noexcept {
  return firstName_;
}

bool User::isActive() const noexcept {
  return active_;
}

} // namespace lms::domain
