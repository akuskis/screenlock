#pragma once

#include "Auth.hpp"

#include <memory>

class ShadowAuth : public Auth
{
public:
    ShadowAuth();
    ~ShadowAuth() override;

    [[nodiscard]] bool is_correct_password(char const* password) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
