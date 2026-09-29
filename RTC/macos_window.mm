#import <AppKit/AppKit.h>
#include <CoreGraphics/CoreGraphics.h>
#include "macos_window.h"

void setMacOSFloatOnTop(QWindow *window) {
    NSView *view = reinterpret_cast<NSView *>(window->winId());
    NSWindow *nsWin = view.window;

    [nsWin setLevel:kCGMaximumWindowLevelKey + 1];

    [nsWin setCollectionBehavior:
        NSWindowCollectionBehaviorCanJoinAllSpaces |
        NSWindowCollectionBehaviorStationary |
        NSWindowCollectionBehaviorIgnoresCycle |
        NSWindowCollectionBehaviorFullScreenAuxiliary];

    [nsWin setHidesOnDeactivate:NO];
}