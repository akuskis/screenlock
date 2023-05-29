#pragma once

class Locker
{
public:
    virtual ~Locker() = default;

    virtual void lock() const = 0;
};
