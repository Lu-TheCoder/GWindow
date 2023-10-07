#import "ApplicationDelegate.h"


static NSString* GetApplicationName(void){
    NSString *appName;
    
    /* Determine the application name */
    appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"];
    if (!appName) {
        appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
    }
    
    if (![appName length]) {
        appName = [[NSProcessInfo processInfo] processName];
    }
    
    return appName;
}

static void CreateApplicationMenus(void)
{
    NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
    
    //App title
    NSMenuItem* appMenuItem;
    NSMenu* appSubmenu;
    NSMenuItem* menuItem;
    NSMenu* appServicesMenu = [[NSMenu alloc] initWithTitle:@"Services"];
    
    appMenuItem = [mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@""];
    appSubmenu = [[NSMenu alloc] initWithTitle:@"Apple"];
    //[NSApp performSelector:NSSelectorFromString(@"setAppleMenu:") withObject:appSubmenu];
    
    // NSString* appName = [[NSProcessInfo processInfo] processName];
    NSString* appName = GetApplicationName();
    menuItem = [appSubmenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", @"About", appName] action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
    
    [appSubmenu addItem:[NSMenuItem separatorItem]];
    
    menuItem = [appSubmenu addItemWithTitle:@"Preferences..." action:nil keyEquivalent:@","];
    
    [appSubmenu addItem:[NSMenuItem separatorItem]];
    
    [appSubmenu addItem:[NSMenuItem separatorItem]];
    menuItem = [appSubmenu addItemWithTitle:@"Services" action:nil keyEquivalent:@""];
    [menuItem setSubmenu:appServicesMenu];
    menuItem = [appServicesMenu addItemWithTitle:@"No Services" action:nil keyEquivalent:@""];
    [NSApp setServicesMenu:appServicesMenu];
    
    [appSubmenu addItem:[NSMenuItem separatorItem]];
    
    menuItem = [appSubmenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", @"Hide", appName] action:@selector(hide:) keyEquivalent:@"h"];
    menuItem = [appSubmenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [menuItem setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
    menuItem = [appSubmenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
    
    menuItem = [appSubmenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", @"Quit", appName] action:@selector(terminate:) keyEquivalent:@"q"];
    
    [mainMenu setSubmenu:appSubmenu forItem:appMenuItem];
    
    //Window
    NSMenuItem* windowMenuItem;
    NSMenu* windowSubmenu;
    
    windowMenuItem = [mainMenu addItemWithTitle:@"Window" action:nil keyEquivalent:@""];
    windowSubmenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    [windowSubmenu addItem:[NSMenuItem separatorItem]];
    
    menuItem = [windowSubmenu addItemWithTitle:@"Minimize" action:@selector(miniaturize:) keyEquivalent:@"m"];
    menuItem = [windowSubmenu addItemWithTitle:@"Minimize All" action:@selector(miniaturizeAll:) keyEquivalent:@"m"];
    [menuItem setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
    menuItem = [windowSubmenu addItemWithTitle:@"Zoom" action:@selector(zoom:) keyEquivalent:@""];
    
    [windowSubmenu addItem:[NSMenuItem separatorItem]];
    
    menuItem = [windowSubmenu addItemWithTitle:@"Bring All to Front" action:@selector(orderFront:) keyEquivalent:@""];
    
    [mainMenu setSubmenu:windowSubmenu forItem:windowMenuItem];
    
    [NSApp setMainMenu:mainMenu];
    [NSApp setWindowsMenu:windowSubmenu];
}


@implementation ApplicationDelegate

-(instancetype)init
{
    self = [super init];
    return self;
}

-(void)applicationWillFinishLaunching:(NSNotification *)notification
{
    
}

-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
    CreateApplicationMenus();   
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    
}


@end
