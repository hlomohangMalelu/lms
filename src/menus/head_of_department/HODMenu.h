#pragma once

#include "service/AuthService.h"
#include "security/BcryptPasswordHasher.h"
#include <SQLiteCpp/SQLiteCpp.h>

namespace lms::menus {

    // Runs the interactive head of department menu.
    void hodMenu(
        SQLite::Database& db,
        const lms::service::AuthenticatedUser& user,
        lms::security::BcryptPasswordHasher& hasher
    );

} // namespace lms::menus
