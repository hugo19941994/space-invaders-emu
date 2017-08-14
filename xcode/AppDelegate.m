#import "AppDelegate.h"
#import "connection.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    _window = [[[NSApplication sharedApplication] windows] firstObject];
}

- (IBAction)openDocument:(id)sender{
    
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    
    [openPanel setAllowedFileTypes:[NSArray arrayWithObjects:@"zip", nil]];
    
    [openPanel beginWithCompletionHandler:^(NSInteger result){
        if (result == NSModalResponseOK) {
            NSURL*  file = [[openPanel URLs] objectAtIndex:0];
            printf("%s", [[file absoluteString] UTF8String]);
            main2((__bridge_retained void *)_window, [[file absoluteString] UTF8String]);
        }
    }];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

@end
