#import "ViewController.h"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    // Update the view, if already loaded.
}

- (void)awakeFromNib {
    // Keep the aspect ratio constant at its current value
    [self.view.window setAspectRatio:self.view.window.frame.size];
}

@end
