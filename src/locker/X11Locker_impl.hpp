#include "X11Locker.hpp"

#include <X11/Xlib.h>

class X11Locker::Impl
{
public:
    explicit Impl(Auth const& auth);
    ~Impl();

    Window drawBlackWindow();
    void drawCursor(Window& window);

    void mapWindow(Window& window);
    void grabKeyboard(Window& window);
    void grabPointer(Window& window);

    void waitForPassword();

private:
    Auth const& auth_;

    _XDisplay* display_;
    int const screen_;
    std::unique_ptr<Pixmap> pixmap_;
    std::unique_ptr<Cursor> cursor_;
};