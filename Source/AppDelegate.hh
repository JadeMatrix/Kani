#import <Cocoa/Cocoa.h>

#import "KaniMainToolbarDelegate.hh"
#import "KaniMainView.hh"


@interface AppDelegate : NSObject< NSApplicationDelegate >
{
    NSWindow               * ns_window;
    KaniMainView           * ns_view;
    NSToolbar              * ns_toolbar;
    // NSToolbarItem          * ns_toolbar_item;
    KaniMainToolbarDelegate* ns_toolbar_delegate;
}

- ( void )applicationDidFinishLaunching : ( NSNotification* )aNotification;
- ( void )populateMainMenu;
- ( void )populateApplicationMenu : ( NSMenu* )aMenu;
// - ( BOOL )applicationShouldTerminateAfterLastWindowClosed : ( NSApplication* )theApplication;

@end
