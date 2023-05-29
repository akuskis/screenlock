#pragma once

#include <stdexcept>

struct LockerError : public std::runtime_error
{
    explicit LockerError(char const* msg)
      : std::runtime_error(msg)
    {
    }
};
