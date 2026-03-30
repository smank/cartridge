#include "FullscreenHelper.h"

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

static IMP sOriginalZoom = nullptr;

static void cartridgeZoom (id self, SEL _cmd, id sender)
{
    NSWindow* window = (NSWindow*) self;

    // If the window supports fullscreen, toggle it.
    // Otherwise fall back to the original zoom behaviour.
    if (([window collectionBehavior] & NSWindowCollectionBehaviorFullScreenPrimary) != 0)
    {
        [window toggleFullScreen: sender];
    }
    else if (sOriginalZoom != nullptr)
    {
        ((void (*)(id, SEL, id)) sOriginalZoom) (self, _cmd, sender);
    }
}

namespace cart {

void enableTitleBarFullscreen (void* nativeWindowHandle)
{
    if (nativeWindowHandle == nullptr)
        return;

    NSView* view = (__bridge NSView*) nativeWindowHandle;
    NSWindow* window = [view window];
    if (window == nil)
        return;

    // Ensure the title is visible and centered (macOS centres by default)
    [window setTitle: @"Cartridge"];
    [window setTitleVisibility: NSWindowTitleVisible];

    // Enable macOS native fullscreen (green traffic-light button + API)
    NSWindowCollectionBehavior behaviour = [window collectionBehavior];
    behaviour |= NSWindowCollectionBehaviorFullScreenPrimary;
    [window setCollectionBehavior: behaviour];

    // Swizzle zoom: on the window's actual class so that
    // double-clicking the title bar triggers fullscreen instead of "zoom".
    static bool swizzled = false;
    if (! swizzled)
    {
        Method zoomMethod = class_getInstanceMethod ([window class], @selector (zoom:));
        if (zoomMethod != nil)
        {
            sOriginalZoom = method_setImplementation (zoomMethod, (IMP) cartridgeZoom);
            swizzled = true;
        }
    }
}

} // namespace cart

#else

namespace cart {
void enableTitleBarFullscreen (void*) {}
}

#endif
