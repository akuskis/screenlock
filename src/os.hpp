#pragma once

namespace os
{
enum class XdgSessionType { X11, WAYLAND, OTHER };

XdgSessionType get_xdg_session_type();

} // namespace os
