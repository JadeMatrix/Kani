#import "AppDelegate.hh"


@implementation AppDelegate

- ( id )init
{
    if( self = [ super init ] )
    {
        // create a reference rect
        NSRect contentSize = NSMakeRect(
            100.0f,
            100.0f,
            800.0f,
            600.0f
        );
        
        // allocate window
        ns_window = [ [ NSWindow alloc ]
            initWithContentRect : contentSize
            styleMask           : (
                NSWindowStyleMaskTitled
                | NSWindowStyleMaskClosable
                | NSWindowStyleMaskMiniaturizable
                | NSWindowStyleMaskResizable
            )
            backing             : NSBackingStoreBuffered
            defer               : YES
        ];
        ns_window.titleVisibility = NSWindowTitleHidden;
        
        ns_toolbar_delegate = [ [ KaniMainToolbarDelegate alloc ] init ];
        
        ns_toolbar = [ [ NSToolbar alloc ]
            initWithIdentifier : @"testToolbar"
        ];
        ns_toolbar.allowsUserCustomization = YES;
        
        ns_toolbar.delegate = ns_toolbar_delegate;
        
        // [ ns_toolbar
        //     insertItemWithItemIdentifier : NSToolbarSeparatorItemIdentifier
        //     atIndex                      : 0
        // ];
        
        ns_window.toolbar = ns_toolbar;
        
        // allocate view
        ns_view = [ [ KaniMainView alloc ] initWithFrame : contentSize ];
    }
    return self;
}

- ( void )applicationWillFinishLaunching : ( NSNotification* )notification
{
    [ ns_window setContentView : ns_view ];
}

- ( void )applicationDidFinishLaunching : ( NSNotification* )aNotification
{
    [ self populateMainMenu ];
    
    // make the window visible.
    [ ns_window makeKeyAndOrderFront : self ];
}

- ( void )dealloc
{
    [ ns_view             release ];
    [ ns_window           release ];
    [ ns_toolbar          release ];
    [ ns_toolbar_delegate release ];
    
    [ super dealloc ];
}

- ( void )populateMainMenu
{
    NSMenu *mainMenu = [ [ NSMenu alloc ] initWithTitle : @"MainMenu" ];
    NSMenuItem *menuItem;
    NSMenu *submenu;
    
    menuItem = [ mainMenu
        addItemWithTitle : @"Apple"
        action : NULL
        keyEquivalent : @""
    ];
    
    submenu = [ [ NSMenu alloc ] initWithTitle : @"Apple" ];
    [ NSApp
        performSelector : NSSelectorFromString( @"setAppleMenu:" )
        withObject : submenu
    ];
    [ self populateApplicationMenu : submenu ];
    
    [ mainMenu
        setSubmenu : submenu
        forItem : menuItem
    ];
    
    [NSApp setMainMenu:mainMenu];
}

- ( void )populateApplicationMenu : ( NSMenu* )aMenu
{
    NSString *applicationName = [ [ NSProcessInfo processInfo ] processName ];
    NSMenuItem *menuItem;
    
    menuItem = [ aMenu
        addItemWithTitle : [ NSString
            stringWithFormat : @"%@ %@",
            NSLocalizedString( @"About", nil ),
            applicationName
        ]
        action           : @selector( orderFrontStandardAboutPanel : )
        keyEquivalent    : @""
    ];
    [ menuItem setTarget : NSApp ];
    
    [ aMenu addItem : [ NSMenuItem separatorItem ] ];
    
    menuItem = [ aMenu
        addItemWithTitle : [ NSString
            stringWithFormat : @"%@ %@",
            NSLocalizedString( @"Quit", nil ),
            applicationName
        ]
        action           : @selector( terminate : )
        keyEquivalent    : @"q"
    ];
    [ menuItem setTarget : NSApp ];
}

// - ( BOOL )applicationShouldTerminateAfterLastWindowClosed : ( NSApplication* )theApplication
// {
//     return YES;
// }

@end
