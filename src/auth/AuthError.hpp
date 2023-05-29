#pragma once

#include <stdexcept>

struct AuthError : public std::runtime_error
{
    explicit AuthError(char const* msg)
      : std::runtime_error(msg)
    {
    }
};
