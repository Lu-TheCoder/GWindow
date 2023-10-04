#pragma once

#import "Platform/platform.h"

@interface WindowDelegate : NSWindow <NSWindowDelegate>
{
    GWindow* handle;
}
-(instancetype)initWithHandle:(GWindow*)windowHandle;

@end //interface WindowDelegate