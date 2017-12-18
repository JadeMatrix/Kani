#import <Cocoa/Cocoa.h>


/******************************************************************************/


@interface KaniView : NSView
{
    NSButton* ns_button;
}

@end


@interface KaniToolbarDelegate : NSObject< NSToolbarDelegate >

- ( NSArray* )toolbarAllowedItemIdentifiers : ( NSToolbar* )toolbar;
- ( NSArray* )toolbarDefaultItemIdentifiers : ( NSToolbar* )toolbar;
- ( NSToolbarItem* )
    toolbar                   : ( NSToolbar* )toolbar 
    itemForItemIdentifier     : ( NSString* )itemIdentifier 
    willBeInsertedIntoToolbar : ( BOOL )flag
;

@end


@interface AppDelegate : NSObject< NSApplicationDelegate >
{
    NSWindow           * ns_window;
    KaniView           * ns_view;
    NSToolbar          * ns_toolbar;
    // NSToolbarItem      * ns_toolbar_item;
    KaniToolbarDelegate* ns_toolbar_delegate;
}

- ( void )applicationDidFinishLaunching : ( NSNotification* )aNotification;
- ( void )populateMainMenu;
- ( void )populateApplicationMenu : ( NSMenu* )aMenu;
// - ( BOOL )applicationShouldTerminateAfterLastWindowClosed : ( NSApplication* )theApplication;

@end


/******************************************************************************/


@implementation KaniToolbarDelegate

// ns_toolbar_item = [ [ NSToolbarItem alloc ]
//     initWithItemIdentifier : @"testToolbarItem"
// ];
// ns_toolbar_item.label = @"toolbar item";
    // [ ns_toolbar_item release ];

- ( NSArray* )toolbarAllowedItemIdentifiers : ( NSToolbar* )toolbar
{
    return [ NSArray
        arrayWithObjects :
            // SaveDocToolbarItemIdentifier,
            // NSToolbarPrintItemIdentifier,
            NSToolbarShowColorsItemIdentifier,
            NSToolbarShowFontsItemIdentifier,
            // NSToolbarCustomizeToolbarItemIdentifier,
            NSToolbarFlexibleSpaceItemIdentifier,
            NSToolbarSpaceItemIdentifier,
            // NSToolbarSeparatorItemIdentifier,
            nil
    ];
}

- ( NSArray* )toolbarDefaultItemIdentifiers : ( NSToolbar* )toolbar
{
    return [ NSArray
        arrayWithObjects :
            NSToolbarShowColorsItemIdentifier,
            NSToolbarShowFontsItemIdentifier,
            NSToolbarFlexibleSpaceItemIdentifier,
            NSToolbarSpaceItemIdentifier,
            nil
    ];
}

- ( NSToolbarItem* )
    toolbar                   : ( NSToolbar* )toolbar 
    itemForItemIdentifier     : ( NSString* )itemIdentifier 
    willBeInsertedIntoToolbar : ( BOOL )flag
{
    // if( itemIdentifier == NSToolbarSeparatorItemIdentifier )
    //     return [ [ NSToolbarSeparatorItem alloc ] init ];
    
    return nil;
}

@end


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
        
        ns_toolbar_delegate = [ [ KaniToolbarDelegate alloc ] init ];
        
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
        ns_view = [ [ KaniView alloc ] initWithFrame : contentSize ];
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


@implementation KaniView

- ( id )initWithFrame : ( NSRect )frame
{
    self = [ super initWithFrame : frame ];
    
    if( self )
    {
        // Initialization code here.
        
        ns_button = [ NSButton
            buttonWithTitle : @"Hello World"
            target          : nil
            action          : nil
        ];
        [ self addSubview : ns_button ];
        
        
    }
    
    return self;
}

- ( void )dealloc
{
    [ ns_button release ];
    [ super dealloc ];
}

- ( void )drawRect : ( NSRect )dirtyRect
{
    [ super drawRect : dirtyRect ];
    
    // Drawing code here.
}

- ( BOOL )acceptsFirstResponder
{
    return YES;
}

// - ( void )mouseDown : ( NSEvent* )anEvent
// {
//     puts( "mousedown!" );
// }

// - ( void )mouseDragged : ( NSEvent* )anEvent
// {
//     puts( "mousemoved!" );
// }

// - ( void )keyDown: ( NSEvent* )anEvent
// {
//     puts( "keydown!" );
// }

@end


/******************************************************************************/


namespace
{
    NSApplication* application;
}


int main( int argc, char* argv[] )
{
    NSAutoreleasePool* pool = [ [ NSAutoreleasePool alloc ] init ];
    
    application = [ NSApplication sharedApplication ];
    
    AppDelegate* applicationDelegate = [ [ [ AppDelegate alloc ] init ]
        autorelease
    ];
    [ application setDelegate : applicationDelegate ];
    [ application run ];
    
    [ pool drain ];
    
    return 0;
}