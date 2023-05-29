#pragma once

#include "Locker.hpp"

#include "auth/Auth.hpp"

#include <memory>

class X11Locker : public Locker
{
public:
    explicit X11Locker(Auth const& auth);
    ~X11Locker() override;

    void lock() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
