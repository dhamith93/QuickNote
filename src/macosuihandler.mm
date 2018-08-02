#include "headers/macosuihandler.h"
#include <QMainWindow>
#include <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>


void MacOSUIHandler::setTitleBar(QMainWindow &w) {

    NSView* view = (NSView*)w.effectiveWinId();
    NSWindow* window = [view window];
    window.titlebarAppearsTransparent = YES;
    [window setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameVibrantDark]];

}
