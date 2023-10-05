#import "ContentView.h"

@implementation ContentView

-(instancetype) initWithHandle:(GWindow*) windowHandle Frame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if(self)
    {
        handle = windowHandle;
        
        //for mouseEnter/Exit
        [self addTrackingRect:frame owner:self userData:nil assumeInside:NO];
    }
    return self;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event {
    return YES;
}

- (void)keyUp:(NSEvent *)event {
}

- (void)mouseDown:(NSEvent *)event {
}

- (void)mouseEntered:(NSEvent *)event {
//    NSLog(@"Mouse Entered");
}

- (void)mouseExited:(NSEvent *)event {
//    NSLog(@"Mouse Left");
}

- (void)mouseMoved:(NSEvent *)event {
}

- (void)mouseUp:(NSEvent *)event {
}

- (void)otherMouseDown:(NSEvent *)event {
}

- (void)otherMouseUp:(NSEvent *)event {
}

- (void)rightMouseDown:(NSEvent *)event {
}

- (void)rightMouseUp:(NSEvent *)event {
}

- (void)scrollWheel:(NSEvent *)event {
}

- (void)keyDown:(NSEvent *)event {
}

@end