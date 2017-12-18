#import "KaniMainToolbarDelegate.hh"


@implementation KaniMainToolbarDelegate

// ns_toolbar_item = [ [ NSToolbarItem alloc ]
//     initWithItemIdentifier : @"testToolbarItem"
// ];
// ns_toolbar_item.label = @"toolbar item";
    // [ ns_toolbar_item release ];

- ( NSArray* )toolbarAllowedItemIdentifiers : ( NSToolbar* )toolbar
{
    return @[
        // SaveDocToolbarItemIdentifier,
        // NSToolbarPrintItemIdentifier,
        NSToolbarShowColorsItemIdentifier,
        NSToolbarShowFontsItemIdentifier,
        // NSToolbarCustomizeToolbarItemIdentifier,
        NSToolbarFlexibleSpaceItemIdentifier,
        NSToolbarSpaceItemIdentifier,
        // NSToolbarSeparatorItemIdentifier
    ];
}

- ( NSArray* )toolbarDefaultItemIdentifiers : ( NSToolbar* )toolbar
{
    return @[
        NSToolbarShowColorsItemIdentifier,
        NSToolbarShowFontsItemIdentifier,
        NSToolbarFlexibleSpaceItemIdentifier,
        NSToolbarSpaceItemIdentifier
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
