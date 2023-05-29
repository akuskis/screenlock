#include "auth/ShadowAuth.hpp"
#include "locker/X11Locker.hpp"
// #include "os.hpp" TODO:

#include <cstdlib>

int main(int /* argc */, char** /* argv */)
{
    ShadowAuth auth;        // TODO: factory to generate correct Auth
    X11Locker locker(auth); // TODO: factory to generate correct Locker

    locker.lock();

    return EXIT_SUCCESS;
}
