#include "X11Locker.hpp"
#include "X11Locker_impl.hpp"


X11Locker::X11Locker(Auth const& auth)
  : impl_(std::make_unique<Impl>(auth))
{
}

X11Locker::~X11Locker() = default;

void X11Locker::lock() const
{
    impl_->lock();
}
