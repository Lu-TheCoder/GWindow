#import "Platform/platform.h"
#import "ApplicationDelegate.h"
#import "WindowDelegate.h"

@class ApplicationDelegate;
@class WindowDelegate;

GWindow* GWindow_create(int width, int height, const char* title, int flags){
    GWindow* window = (GWindow*)malloc(sizeof(GWindow));

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
        
        [window->window setLevel:NSNormalWindowLevel];
        // [window->window setBackgroundColor: NSColor.redColor];
        [window->window setTitle: @(title)];
        [window->window setDelegate:window->window_delegate];
        [window->window.contentView setWantsLayer: YES];
        [window->window setAcceptsMouseMovedEvents: YES];
        [window->window setRestorable:NO];
        [window->window makeKeyAndOrderFront: nil];
        
        window->layer = [CAMetalLayer layer];
        
        NSLog(@"Width: %f", window->window.contentView.frame.size.height);
        
//        window->layer.frame = window->window.contentView.frame;
        window->layer.bounds = window->window.contentView.bounds;
        window->layer.drawableSize = [window->window.contentView convertSizeToBacking:window->window.contentView.bounds.size];
        window->layer.contentsScale = window->window.contentView.window.backingScaleFactor;
        
        window->layer.pixelFormat = MTLPixelFormatRGBA8Unorm;
        
//        [window->layer setDevice:MTLCreateSystemDefaultDevice()];
        [window->window.contentView setLayerContentsRedrawPolicy:NSViewLayerContentsRedrawDuringViewResize];
        
        [window->window.contentView setLayer: window->layer];

        
        [window->layer setOpaque: YES];
        
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
    
}

bool GWindow_should_close(GWindow* window){
    return window->should_close;
}
