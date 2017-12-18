#import "KaniMainView.hh"


@implementation KaniMainView

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
