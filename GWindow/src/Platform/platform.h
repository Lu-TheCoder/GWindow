#pragma once
#include "defines.h"

#if defined(GPLATFORM_APPLE)
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@class ApplicationDelegate;
@class WindowDelegate;

#endif


typedef struct GWindow GWindow;

typedef struct GWindow {
    NSWindow* window;
    CAMetalLayer* layer;
    ApplicationDelegate* app_delegate;
    WindowDelegate* window_delegate;
    bool should_close;
    bool isResized;
}GWindow;

G_API GWindow* GWindow_create(int width, int height, const char* title, int flags);
G_API void GWindow_destroy(GWindow* window);
G_API void GWindow_poll_events(void);
G_API bool GWindow_should_close(GWindow* window);
G_API void GWindow_setLayer_device(GWindow* window, id<MTLDevice> device);
G_API void GWindow_getRequiredVulkanExtensions(u32* out_count, const char** pExtensions);
