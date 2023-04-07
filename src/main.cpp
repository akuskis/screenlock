#include "os.hpp"

#include <cstdlib>
#include <iostream>

int main(int /* argc */, char** /* argv */)
{
    auto const xdg_session_type = os::get_xdg_session_type();

    std::cout << (int)xdg_session_type << std::endl;

    // auto window_system = Factory::create(current_window_system);
    // window_system.lock()

    return EXIT_SUCCESS;
}
