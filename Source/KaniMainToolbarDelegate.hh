#import <Cocoa/Cocoa.h>


@interface KaniMainToolbarDelegate : NSObject< NSToolbarDelegate >

- ( NSArray* )toolbarAllowedItemIdentifiers : ( NSToolbar* )toolbar;
- ( NSArray* )toolbarDefaultItemIdentifiers : ( NSToolbar* )toolbar;
- ( NSToolbarItem* )
    toolbar                   : ( NSToolbar* )toolbar 
    itemForItemIdentifier     : ( NSString* )itemIdentifier 
    willBeInsertedIntoToolbar : ( BOOL )flag
;

@end
