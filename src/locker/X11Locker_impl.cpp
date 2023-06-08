#include "X11Locker_impl.hpp"

#include "LockerError.hpp"
#include "Position.hpp"
#include "cursor.bitmap"
#include "cursor_text.bitmap"
#include "mask.bitmap"

#include <X11/X.h>
#include <X11/Xutil.h>

#include <cmath>
#include <thread>

namespace
{
const int FULL_CIRCLE = 360;
const auto UPDATE_DELAY_MSEC = std::chrono::milliseconds(20);

const int PIXMAP_POINT_SIZE = sizeof(cursor_bits[0]) * 8;
const int ICON_PIXMAP_SIZE = cursor_width * cursor_height / PIXMAP_POINT_SIZE;


_XDisplay* get_display()
{
    auto display = XOpenDisplay(nullptr);

    if (!display)
        throw LockerError("Cannot open display");

    return display;
}

Size<unsigned int> get_window_geometry(Display* display, Window& window)
{
    Window root_window;
    int x, y;
    unsigned int width, height, border, depth;

    XGetGeometry(display, window, &root_window, &x, &y, &width, &height, &border, &depth);

    return {width, height};
}

Position<int> getCursorPosition(Display* display, Window& window)
{
    Window window_returned;
    int x, y, win_x, win_y;
    unsigned int mask_return;

    XQueryPointer(display, window, &window_returned, &window_returned, &x, &y, &win_x, &win_y, &mask_return);

    return {x, y};
}


double to_rad(int const angle)
{
    return angle * M_PI / 180.0;
}

Pixmap prepare_image(Display* display, Window& window, unsigned int angle_degrees)
{
    std::array<uint16_t, ICON_PIXMAP_SIZE> img = {0};
    double rad = -to_rad(static_cast<int>(angle_degrees));

    for (int x = 0; x < cursor_text_x_hot * 2; ++x)
    {
        for (int y = 0; y < cursor_text_y_hot * 2; ++y)
        {
            int const rx
              = (x - cursor_text_x_hot) * cos(rad) - (y - cursor_text_y_hot) * sin(rad) + cursor_text_x_hot + 0.5;
            int const ry
              = (x - cursor_text_x_hot) * sin(rad) + (y - cursor_text_y_hot) * cos(rad) + cursor_text_y_hot + 0.5;

            int from = (cursor_width / PIXMAP_POINT_SIZE * y) + x / PIXMAP_POINT_SIZE;
            int to = (cursor_width / PIXMAP_POINT_SIZE * ry) + rx / PIXMAP_POINT_SIZE;

            if (0 <= from && from <= ICON_PIXMAP_SIZE && 0 <= to && to < ICON_PIXMAP_SIZE && 0 <= rx
                && rx < cursor_width && 0 <= ry && ry < cursor_width)
                img[to] |= ((cursor_text_bits[from] >> (x % PIXMAP_POINT_SIZE)) & 0x01) << (rx % PIXMAP_POINT_SIZE);
        }
    }

    for (int i = 0; i < ICON_PIXMAP_SIZE; ++i)
    {
        img[i] &= mask_bits[i];
        img[i] |= cursor_bits[i];
    }

    return XCreateBitmapFromData(display, window, (char*)img.data(), cursor_width, cursor_height);
}
} // namespace


X11Locker::Impl::Impl(Auth const& auth)
  : auth_(auth)
  , display_(get_display())
  , screen_(DefaultScreen(display_))
{
    icons_.reserve(FULL_CIRCLE);
}

X11Locker::Impl::~Impl()
{
    XUngrabKeyboard(display_, CurrentTime);

    for (auto const& icon : icons_)
        XFreePixmap(display_, icon);

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

void X11Locker::Impl::hideSystemCursor(Window& window)
{
    Pixmap blank_pixmap = XCreatePixmap(display_, window, 1, 1, 1);
    XColor color;
    Cursor blank_cursor = XCreatePixmapCursor(display_, blank_pixmap, blank_pixmap, &color, &color, 0, 0);
    XDefineCursor(display_, window, blank_cursor);
    XFreeCursor(display_, blank_cursor);
    XFreePixmap(display_, blank_pixmap);
}

void X11Locker::Impl::updateScene(Window& window, unsigned int angle, Size<unsigned int> window_size)
{
    while (icons_.size() <= angle)
        icons_.push_back(prepare_image(display_, window, angle));

    auto const pos = getCursorPosition(display_, window);

    auto pixmap = XCreatePixmap(display_,
                                window,
                                window_size.getWidth(),
                                window_size.getHeight(),
                                DefaultDepth(display_, DefaultScreen(display_)));
    auto gc = XCreateGC(display_, pixmap, 0, nullptr);

    XFillRectangle(display_, pixmap, gc, 0, 0, window_size.getWidth(), window_size.getHeight());

    XColor color;
    XAllocNamedColor(display_, DefaultColormap(display_, DefaultScreen(display_)), "steelblue3", &color, &color);
    XSetForeground(display_, gc, color.pixel);
    XCopyPlane(display_,
               icons_[angle],
               pixmap,
               gc,
               0,
               0,
               cursor_width,
               cursor_height,
               pos.getX() - cursor_x_hot,
               pos.getY() - cursor_y_hot,
               1);

    XCopyArea(display_, pixmap, window, gc, 0, 0, window_size.getWidth(), window_size.getHeight(), 0, 0);

    XFreePixmap(display_, pixmap);
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

void X11Locker::Impl::eventLoop(Window& window)
{
    XEvent event;
    KeySym key;
    char buffer[10];
    char password[128];
    size_t password_offset = 0;
    int icon_angle = 0;

    auto const window_size = get_window_geometry(display_, window);

    while (true)
    {
        if (!XPending(display_))
        {
            icon_angle = (icon_angle + 1) % FULL_CIRCLE;
            updateScene(window, icon_angle, window_size);
            std::this_thread::sleep_for(UPDATE_DELAY_MSEC);
        }
        else
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
}

void X11Locker::Impl::lock()
{
    auto window = drawBlackWindow();

    hideSystemCursor(window);
    mapWindow(window);
    grabKeyboard(window);

    eventLoop(window);
}
