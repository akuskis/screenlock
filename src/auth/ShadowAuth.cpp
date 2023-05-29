#include "ShadowAuth.hpp"

#include "AuthError.hpp"

#include <cstring>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>

namespace
{
int const EQUAL_STRINGS = 0;
}

struct ShadowAuth::Impl
{
    spwd* shadow_entry = nullptr;
};


ShadowAuth::ShadowAuth()
  : impl_(std::make_unique<Impl>())
{
    auto* pw = getpwuid(getuid());
    if (!pw)
        throw AuthError("Password entry for uid not found");

    impl_->shadow_entry = getspnam(pw->pw_name);
    if (!impl_->shadow_entry)
        throw AuthError((std::string("Failed to read shadow entry for user ") + pw->pw_name).c_str());

    if (setgid(getgid()))
        throw AuthError("Can't set group ID");

    if (setuid(getuid()))
        throw AuthError("Can't set user ID");
}

ShadowAuth::~ShadowAuth() = default;

bool ShadowAuth::is_correct_password(char const* password) const
{
    return strcmp(impl_->shadow_entry->sp_pwdp, crypt(password, impl_->shadow_entry->sp_pwdp)) == EQUAL_STRINGS;
}
