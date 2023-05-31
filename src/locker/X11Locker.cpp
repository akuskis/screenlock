#include "X11Locker.hpp"
#include "X11Locker_impl.hpp"


X11Locker::X11Locker(Auth const& auth)
  : impl_(std::make_unique<Impl>(auth))
{
}

X11Locker::~X11Locker() = default;

void X11Locker::lock() const
{
    auto window = impl_->drawBlackWindow();

    impl_->drawCursor(window);
    impl_->mapWindow(window);
    impl_->grabKeyboard(window);
    impl_->grabPointer(window);

    impl_->waitForPassword();
}
