#pragma once
#include "security/IAuthorizationPolicy.h"
namespace test_support {
class AllowAuthorizationPolicy final : public lms::security::IAuthorizationPolicy {
public:
 void requireAdministrator(long long) const override {}
 void requireCanSubmit(long long, long long) const override {}
 void requireCanGrantExtension(long long, long long, long long) const override {}
 void requireCanGrantClassExtension(long long, long long) const override {}
 void requireCanMark(long long, long long) const override {}
 void requireCanModerate(long long, long long) const override {}
};
}
