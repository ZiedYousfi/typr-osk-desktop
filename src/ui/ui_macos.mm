// ui_macos.mm
// macOS-specific UI utilities (Objective-C++)

#include "ui.hpp"

#ifdef __APPLE__

#import <Cocoa/Cocoa.h>
#import <objc/message.h>
#import <objc/runtime.h>

void Ui::makeNonActivating(QWidget *w) {
  if (!w || !w->window())
    return;

  // Set the application as an accessory app (like menu bar apps)
  // This prevents it from appearing in Dock and Cmd+Tab, and limits activation
  [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

  // Ensure the window is created
  w->winId();

  NSView *nsView =
      (__bridge NSView *)reinterpret_cast<void *>(w->window()->winId());
  if (!nsView)
    return;

  NSWindow *nsWindow = [nsView window];
  if (!nsWindow)
    return;

  // Check if we already processed this window
  static const char *processedKey = "typr_processed";
  if (objc_getAssociatedObject(nsWindow, processedKey)) {
    return;
  }
  objc_setAssociatedObject(nsWindow, processedKey, @YES, OBJC_ASSOCIATION_RETAIN);

  // Create a unique subclass for this window instance
  Class originalClass = object_getClass(nsWindow);
  NSString *newClassName = [NSString stringWithFormat:@"TyprNonActivating_%p", nsWindow];
  Class newClass = objc_allocateClassPair(originalClass, [newClassName UTF8String], 0);

  if (newClass) {
    // Override canBecomeKeyWindow to return NO
    IMP canBecomeKeyIMP = imp_implementationWithBlock(^BOOL(id self) {
      return NO;
    });
    class_addMethod(newClass, @selector(canBecomeKeyWindow), canBecomeKeyIMP, "B@:");

    // Override canBecomeMainWindow to return NO
    IMP canBecomeMainIMP = imp_implementationWithBlock(^BOOL(id self) {
      return NO;
    });
    class_addMethod(newClass, @selector(canBecomeMainWindow), canBecomeMainIMP, "B@:");

    // Override acceptsFirstResponder to return NO
    IMP acceptsFirstResponderIMP = imp_implementationWithBlock(^BOOL(id self) {
      return NO;
    });
    class_addMethod(newClass, @selector(acceptsFirstResponder), acceptsFirstResponderIMP, "B@:");

    // Override becomeKeyWindow to do nothing
    IMP becomeKeyWindowIMP = imp_implementationWithBlock(^(id self) {
      // Do nothing - refuse to become key
    });
    class_addMethod(newClass, @selector(becomeKeyWindow), becomeKeyWindowIMP, "v@:");

    // Override becomeMainWindow to do nothing
    IMP becomeMainWindowIMP = imp_implementationWithBlock(^(id self) {
      // Do nothing - refuse to become main
    });
    class_addMethod(newClass, @selector(becomeMainWindow), becomeMainWindowIMP, "v@:");

    // Override _
    IMP preventsActivationIMP = imp_implementationWithBlock(^BOOL(id self) {
      return YES;
    });
    class_addMethod(newClass, @selector(_preventsActivation), preventsActivationIMP, "B@:");

    objc_registerClassPair(newClass);
    object_setClass(nsWindow, newClass);
  }

  // Set the window style mask to include NSWindowStyleMaskNonactivatingPanel behavior
  NSWindowStyleMask currentMask = [nsWindow styleMask];
  [nsWindow setStyleMask:currentMask | NSWindowStyleMaskNonactivatingPanel];

  // Set collection behavior
  [nsWindow setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces |
                                  NSWindowCollectionBehaviorFullScreenAuxiliary |
                                  NSWindowCollectionBehaviorStationary |
                                  NSWindowCollectionBehaviorIgnoresCycle];

  // Use a floating window level
  [nsWindow setLevel:NSFloatingWindowLevel];

  // Don't hide when app is deactivated
  [nsWindow setHidesOnDeactivate:NO];

  // Disable animations
  [nsWindow setAnimationBehavior:NSWindowAnimationBehaviorNone];

  // Make sure we can receive mouse events
  [nsWindow setIgnoresMouseEvents:NO];

  // Force resign if somehow key
  if ([nsWindow isKeyWindow]) {
    [nsWindow resignKeyWindow];
  }
  if ([nsWindow isMainWindow]) {
    [nsWindow resignMainWindow];
  }
}

#endif // __APPLE__
