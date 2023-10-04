#import "WindowDelegate.h"

@implementation WindowDelegate


- (instancetype)initWithHandle:(GWindow *)windowHandle{
    
    self = [super init];
    
    if(self != nil){
        handle = windowHandle;
        handle->should_close = false;
    }
    
    return self;
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    handle->should_close = true;
    return YES;
}

- (void)windowDidResize:(NSNotification *)notification
{
    
    @autoreleasepool {
        CGSize viewSize = handle->window.contentView.bounds.size;
        NSSize newDrawableSize = [handle->window.contentView convertSizeToBacking:viewSize];
        
        handle->layer.drawableSize = newDrawableSize;
        handle->layer.contentsScale = handle->window.contentView.window.backingScaleFactor;
        
        //TODO: dont know if it belongs here..
        //    [window.contentView setLayer: handle->layer];
        
        handle->isResized = true;
    }
    
    // NSLog(@"Did Resize");
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    
}

@end
