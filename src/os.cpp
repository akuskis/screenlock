#include "os.hpp"

#include <cstdlib>
#include <string>
#include <unordered_map>

namespace os
{
namespace
{
std::unordered_map<std::string, XdgSessionType> const xdg_session_map{{"x11", XdgSessionType::X11},
                                                                      {"wayland", XdgSessionType::WAYLAND}};
} // namespace

XdgSessionType get_xdg_session_type()
{
    auto it = xdg_session_map.find(std::getenv("XDG_SESSION_TYPE"));

    return it == xdg_session_map.end() ? XdgSessionType::OTHER : it->second;
}
} // namespace os
