// ui_macos.mm
// macOS-specific UI utilities (Objective-C++)

#include "ui.hpp"

#ifdef __APPLE__

#import <Cocoa/Cocoa.h>

void Ui::makeNonActivating(QWidget *w) {
  if (!w || !w->window())
    return;

  NSView *nsView =
      (__bridge NSView *)reinterpret_cast<void *>(w->window()->winId());
  if (!nsView)
    return;

  NSWindow *nsWindow = [nsView window];
  if (!nsWindow)
    return;

  // Allow mouse events
  [nsWindow setIgnoresMouseEvents:NO];

  // Set collection behavior for overlay-like windows
  [nsWindow
      setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces |
                            NSWindowCollectionBehaviorFullScreenAuxiliary];

  // Keep window above others
  [nsWindow setLevel:NSStatusWindowLevel];

  // Don't hide when app is deactivated
  [nsWindow setHidesOnDeactivate:NO];
}

#endif // __APPLE__
