#pragma once

#include "domain/User.h"

#include <optional>

namespace lms::repository {

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    [[nodiscard]] virtual std::optional<domain::User>
    findById(domain::UserId id) const = 0;
};

} // namespace lms::repository
