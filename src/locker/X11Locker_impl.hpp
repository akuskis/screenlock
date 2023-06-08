#include "Size.hpp"
#include "X11Locker.hpp"

#include <X11/Xlib.h>

#include <vector>


class X11Locker::Impl
{
public:
    explicit Impl(Auth const& auth);
    ~Impl();

    void lock();

private:
    Auth const& auth_;
    _XDisplay* display_;
    int const screen_;

    std::vector<Pixmap> icons_;

    Window drawBlackWindow();

    void hideSystemCursor(Window& window);
    void updateScene(Window& window, unsigned int angle, Size<unsigned int> window_size);
    void mapWindow(Window& window);
    void grabKeyboard(Window& window);

    void eventLoop(Window& window);
};
