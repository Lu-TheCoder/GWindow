#import "Platform/platform.h"
#import "ApplicationDelegate.h"
#import "WindowDelegate.h"
#import "ContentView.h"
#include <stdlib.h>
#include <string.h>
#include "Containers/dynlist.h"

@class ApplicationDelegate;
@class WindowDelegate;


static GWindow* window;

GWindow* GWindow_create(int width, int height, const char* title, int flags){
    window = (GWindow*)malloc(sizeof(GWindow));

    @autoreleasepool {
        
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];
        
        window->app_delegate = [[ApplicationDelegate alloc] init];
        [NSApp setDelegate: window->app_delegate];
        
        window->window_delegate = [[WindowDelegate alloc] initWithHandle:window];
        
        NSRect screenRect = [NSScreen mainScreen].frame;
        NSRect initialFrame = NSMakeRect((screenRect.size.width - width) * 0.5f,
                                         (screenRect.size.height - height) * 0.5f,
                                         width, height);
        
        NSWindowStyleMask windowStyleMask = (NSWindowStyleMaskTitled |
                                             NSWindowStyleMaskClosable |
                                             NSWindowStyleMaskMiniaturizable |
                                             NSWindowStyleMaskResizable);
        
        
        window->window = [[NSWindow alloc] initWithContentRect:initialFrame
                                                     styleMask:windowStyleMask
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];

        ContentView* cView = [[ContentView alloc] initWithHandle:window Frame:initialFrame];

        // Layer creation
        window->handle.layer = [CAMetalLayer layer];
        if (!window->handle.layer) {
            printf("Failed to create layer for view.\n");
        }
        
        
        [window->window setLevel:NSNormalWindowLevel];
        // [window->window setBackgroundColor: NSColor.redColor];
        [window->window setContentView: cView];
        [window->window makeFirstResponder: cView];
        [window->window setTitle: @(title)];
        [window->window setDelegate:window->window_delegate];
        [window->window setAcceptsMouseMovedEvents: YES];
        [window->window setRestorable:NO];
        [window->window makeKeyAndOrderFront: nil];

        window->handle.layer.bounds = cView.bounds;
        window->handle.layer.drawableSize = [window->window.contentView convertSizeToBacking:window->window.contentView.bounds.size];
        window->handle.layer.contentsScale = window->window.contentView.window.backingScaleFactor;
        printf("contentScale: %f", window->handle.layer.contentsScale);

        [window->window.contentView setLayer: window->handle.layer];
        window->handle.layer.opaque = YES;

        // window->layer = [CAMetalLayer layer];
        // // [window->layer setDevice:MTLCreateSystemDefaultDevice()];
        // window->layer.frame = window->window.contentView.frame;
        // window->layer.bounds = window->window.contentView.bounds;
        // window->layer.drawableSize = [window->window.contentView convertSizeToBacking:window->window.contentView.bounds.size];
        // window->layer.contentsScale = window->window.contentView.window.backingScaleFactor;
        
        // // window->layer.pixelFormat = MTLPixelFormatRGBA8Unorm;
        
        // [window->layer setOpaque: YES];
        
        // [cView setWantsLayer:YES];
        // [cView setLayer:window->layer];
        
        // [window->window setContentView: cView];
        // [window->window.contentView setLayerContentsRedrawPolicy:NSViewLayerContentsRedrawDuringViewResize];
        
        NSLog(@"Width: %f", window->window.contentView.frame.size.height);
        
        [NSApp finishLaunching];
        
    } //autorelease
    
    return window;
}

void GWindow_setLayer_device(GWindow* window, id<MTLDevice> device){
    window->layer.device = device;
}

void GWindow_poll_events(void){
    @autoreleasepool {
        
        for(;;)
        {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event == nil)
            {
                break;
            }
            
            [NSApp sendEvent:event];
        }
        
    }    //autoreleasepool
}

void GWindow_destroy(GWindow* window){
    free(window);
}

bool GWindow_should_close(GWindow* window){
    return window->should_close;
}

void GWindow_getRequiredVulkanExtensions(const char*** pExtensions){
    dynlist_push(*pExtensions, &"VK_KHR_surface");
    dynlist_push(*pExtensions, &"VK_EXT_metal_surface");
    dynlist_push(*pExtensions, &"VK_KHR_portability_enumeration");
}

static void platform_copy_memory(void *dest, const void *source, u64 size){
    memcpy(dest, source, size);
}

void GWindow_get_handle_info(u64* out_size, void* memory){
    *out_size = sizeof(macos_handle_info);
    if(!memory){
        return;
    }

    platform_copy_memory(memory, &window->handle, *out_size);
    // memcpy(memory, (const void*)(window->handle), *out_size);
}