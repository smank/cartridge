#pragma once

namespace cart {

/// On macOS, overrides the title bar double-click ("zoom") to toggle
/// native fullscreen instead.  No-op on other platforms.
void enableTitleBarFullscreen (void* nativeWindowHandle);

} // namespace cart
