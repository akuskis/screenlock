#include "X11Locker.hpp"

#include "LockerError.hpp"
#include "cursor.bitmap"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


namespace
{
_XDisplay* get_display()
{
    auto display = XOpenDisplay(nullptr);

    if (!display)
        throw LockerError("Cannot open display");

    return display;
}

} // namespace

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

X11Locker::Impl::Impl(Auth const& auth)
  : auth_(auth)
  , display_(get_display())
  , screen_(DefaultScreen(display_))
{
}

X11Locker::Impl::~Impl()
{
    if (cursor_)
        XFreeCursor(display_, *cursor_);

    if (pixmap_)
        XFreePixmap(display_, *pixmap_);

    XUngrabKeyboard(display_, CurrentTime);
    XUngrabPointer(display_, CurrentTime);
    XCloseDisplay(display_);
}

Window X11Locker::Impl::drawBlackWindow()
{
    XSetWindowAttributes attrib;

    attrib.override_redirect = True;
    attrib.background_pixel = BlackPixel(display_, screen_);

    return XCreateWindow(display_,
                         DefaultRootWindow(display_),
                         0,
                         0,
                         DisplayWidth(display_, screen_),
                         DisplayHeight(display_, screen_),
                         0,
                         DefaultDepth(display_, screen_),
                         CopyFromParent,
                         DefaultVisual(display_, screen_),
                         CWOverrideRedirect | CWBackPixel,
                         &attrib);
}

void X11Locker::Impl::drawCursor(Window& window)
{
    XColor color;
    XAllocNamedColor(display_, DefaultColormap(display_, DefaultScreen(display_)), "steelblue3", &color, &color);

    pixmap_ = std::make_unique<Pixmap>(
      XCreateBitmapFromData(display_, window, (char*)cursor_bits, cursor_width, cursor_height));

    cursor_ = std::make_unique<Cursor>(
      XCreatePixmapCursor(display_, *pixmap_, *pixmap_, &color, &color, cursor_x_hot, cursor_y_hot));
}

void X11Locker::Impl::mapWindow(Window& window)
{
    XMapWindow(display_, window);
}

void X11Locker::Impl::grabKeyboard(Window& window)
{
    // try to grab keyboard multiple times (this is necessary when application is executed from shortcut)
    for (size_t i = 0; i < 100; ++i)
    {
        if (XGrabKeyboard(display_, window, False, GrabModeAsync, GrabModeAsync, CurrentTime) == GrabSuccess)
            return;

        timeval tv{.tv_sec = 0, .tv_usec = 10000};
        select(1, nullptr, nullptr, nullptr, &tv);
    }

    throw LockerError("Cannot grab keyboard");
}

void X11Locker::Impl::grabPointer(Window& window)
{
    if (!cursor_
        || XGrabPointer(display_,
                        window,
                        False,
                        (KeyPressMask | KeyReleaseMask) & 0,
                        GrabModeAsync,
                        GrabModeAsync,
                        None,
                        *cursor_,
                        CurrentTime)
             != GrabSuccess)
        throw LockerError("Cannot grab pointer");
}

void X11Locker::Impl::waitForPassword()
{
    XEvent event;
    KeySym key;
    char buffer[10];
    char password[128];
    size_t password_offset = 0;

    for (;;)
    {
        XNextEvent(display_, &event);
        switch (event.type)
        {
        case KeyPress: {
            auto input_len = XLookupString(&event.xkey, buffer, sizeof(buffer) - 1, &key, nullptr);

            switch (key)
            {
            case XK_BackSpace:
                if (password_offset != 0)
                    --password_offset;
                break;
            case XK_Escape:
                password_offset = 0;
                break;
            case XK_Return:
            case XK_KP_Enter:
                password[password_offset] = '\0';
                password_offset = 0;
                if (auth_.is_correct_password(password))
                    return;
                break;
            default:
                if (input_len == 1)
                    password[password_offset++] = buffer[0];

                password_offset %= sizeof(password);
                break;
            }
            break;
        }
        default:
            break;
        }
    }
}

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
