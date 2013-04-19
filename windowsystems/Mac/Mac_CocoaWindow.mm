/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Cocoa/Cocoa.h>
#import "Mac_CocoaWindow.h"
#include "SystemDelegate.h"
#include "Mac_LoadingOverlay.h"
#include "Mac_Window.h"
#include "Mac_Keyboard.h"

extern bool gReattachLoadingOverlay;

@implementation AprilCocoaWindow

- (void)timerEvent:(NSTimer*) t
{
	[mView setNeedsDisplay:YES];
}

- (void)configure
{
	april::initMacKeyMap();
	
	[self setBackgroundColor:[NSColor blackColor]];
	[self setOpaque:YES];
	[self setDelegate:self];
	[self setAcceptsMouseMovedEvents:YES];
}

- (void)startRenderLoop
{
	mTimer = [NSTimer timerWithTimeInterval:0.00000001f target:self selector:@selector(timerEvent:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:mTimer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:mTimer forMode:NSEventTrackingRunLoopMode];
}

- (BOOL)windowShouldClose:(NSWindow*) sender
{
	if (aprilWindow->handleQuitRequest(true))
	{
		[NSApp terminate:nil];
		return YES;
	}
	else
	{
		NSLog(@"Aborting window close request per app's request.");
	}
	return NO;
}

- (void)onWindowSizeChange
{
	april::SystemDelegate* delegate = aprilWindow->getSystemDelegate();
	NSSize size = [mView bounds].size;
	[mView updateGLViewport];
	if ([self inLiveResize])
	{
		mWindowedRect = [self frame];
	}
	
	if (delegate)
	{
		delegate->onWindowSizeChanged(size.width, size.height, april::Window::ADEVICEORIENTATION_NONE);
	}
	[mView setNeedsDisplay:YES];
}

- (void)windowDidResize:(NSNotification*) notification
{
	[self onWindowSizeChange];
	updateLoadingOverlay(0);
}

- (BOOL)isFullScreen
{
	int style = [self styleMask];
	return (style == NSBorderlessWindowMask || style == NSFullScreenWindowMask); // this covers both 10.7 and older macs
}

- (void)enterFullScreen
{
	aprilWindow->setFullscreenFlag(1);
	NSRect prevFrame = [self frame];
	[self setStyleMask:NSBorderlessWindowMask];
	[self setFrame: [[NSScreen mainScreen] frame] display:YES];
	[[NSApplication sharedApplication] setPresentationOptions: NSFullScreenWindowMask | NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideDock | NSApplicationPresentationDisableMenuBarTransparency];
	if (!NSEqualRects(prevFrame, [self frame]))
	{
		[self onWindowSizeChange];
	}
}

- (void)windowWillEnterFullScreen:(NSNotification*) notification
{
	[self enterFullScreen]; // this gets called on 10.7+, while < 10.7 call enterFullScreen directly
}

- (void)setWindowedStyleMask
{
	NSUInteger mask;
	mask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
	if (aprilWindow->mResizable) mask |= NSResizableWindowMask;
	[self setStyleMask:mask];
	if (getMacOSVersion() >= 10.7f) // workarround for bug in macos
	{
		[[self standardWindowButton:NSWindowMiniaturizeButton] setEnabled:YES];
	}
}

- (void)exitFullScreen
{
	NSRect prevFrame = [self frame];
	[[NSApplication sharedApplication] setPresentationOptions: NSApplicationPresentationDefault];
	
	NSRect rect = mWindowedRect; // in case mWindowedRect gets overriden in onWindowResize
	[self setWindowedStyleMask];
	[self setFrame:rect display:YES];
	mWindowedRect = rect;
	
	if (!NSEqualRects(prevFrame, [self frame]))
	{
		[self onWindowSizeChange];
	}
	aprilWindow->setFullscreenFlag(0);
}

- (void)windowWillExitFullScreen:(NSNotification*) notification
{
	[self setWindowedStyleMask];
}

- (void)windowDidExitFullScreen:(NSNotification*) notification
{
	[[NSApplication sharedApplication] setPresentationOptions: NSApplicationPresentationDefault];

	[self setWindowedStyleMask];
	[self setFrame:mWindowedRect display:YES];

}

- (gvec2)transformCocoaPoint:(NSPoint) point
{
	// TODO: optimize
	gvec2 pt(point.x, aprilWindow->getHeight() - point.y);
	return pt;
}

- (april::Key)getMouseButtonCode:(NSEvent*) event
{
	april::Key button;
	int n = event.buttonNumber;
	if      (n == 0) button = april::AK_LBUTTON;
	else if (n == 1) button = april::AK_RBUTTON;
	else             button = april::AK_MBUTTON;
	return button;
}

- (void)mouseDown:(NSEvent*) event
{	
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
	aprilWindow->handleMouseEvent(april::Window::AMOUSEEVT_DOWN, pos, [self getMouseButtonCode:event]);
}

- (void)rightMouseDown:(NSEvent*) event
{
	[self mouseDown:event];
}

- (void)otherMouseDown:(NSEvent*) event
{
	[self mouseDown:event];	
}

- (void)mouseUp:(NSEvent*) event
{
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
	aprilWindow->handleMouseEvent(april::Window::AMOUSEEVT_UP, pos, [self getMouseButtonCode:event]);	
}

- (void)rightMouseUp:(NSEvent*) event
{
	[self mouseUp:event];
}

- (void)otherMouseUp:(NSEvent*) event
{
	[self mouseUp:event];
}

- (void)mouseMoved:(NSEvent*) event
{
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
	aprilWindow->handleMouseEvent(april::Window::AMOUSEEVT_MOVE, pos, april::AK_NONE);
}

- (void)mouseDragged:(NSEvent*) event
{
	[self mouseMoved:event];
}

- (void)rightMouseDragged:(NSEvent*) event
{
	[self mouseMoved:event];
}

- (void)otherMouseDragged:(NSEvent*) event
{
	[self mouseMoved:event];
}

- (void)onKeyDown:(unsigned int) keyCode unicode:(NSString*) unicode
{
	unsigned int unichr = 0;
	if ([unicode length] > 0)
	{
		unichr = [unicode characterAtIndex:0];
	}
	aprilWindow->handleKeyEvent(april::Window::AKEYEVT_DOWN, (april::Key) keyCode, unichr);
}

- (void)onKeyUp:(unsigned int) keyCode
{
	aprilWindow->handleKeyEvent(april::Window::AKEYEVT_UP, (april::Key) keyCode, 0);
}

- (unsigned int) processKeyCode:(NSEvent*) event
{
	NSString* chr = [event characters];
	if ([chr length] == 1)
	{
		unichar c = [chr characterAtIndex:0];
		if (c >= 'a' && c <= 'z') return toupper(c);
	}
	return april::getAprilMacKeyCode([event keyCode]);
}

- (void)platformToggleFullScreen
{
	if (getMacOSVersion() >= 10.7f)
	{
		[self toggleFullScreen:self];
	}
	else
	{
		if ([self isFullScreen])
			[self exitFullScreen];
		else
			[self enterFullScreen];
	}
}

- (void)keyDown:(NSEvent*) event
{
	if (event.keyCode != 53) [super keyDown:event]; // prevent forwarding ESC key to super class because Lion uses it to exit fullscreen mode
	if ((event.modifierFlags & (NSCommandKeyMask | NSControlKeyMask)) == (NSCommandKeyMask | NSControlKeyMask))
	{
		NSString* s = [event charactersIgnoringModifiers];
		if ([s isEqualTo:@"f"]) // fullscreen toggle
		{
			[self platformToggleFullScreen];
			return;
		}
	}
	[self onKeyDown:[self processKeyCode:event] unicode:[event characters]];
}

- (void)keyUp:(NSEvent*) event
{
	if (event.keyCode != 53) [super keyUp:event]; // prevent forwarding ESC key to super class because Lion uses it to exit fullscreen mode
	if (event.modifierFlags & NSCommandKeyMask)
	{
		NSString* s = [event characters];
		if ([s isEqualTo:@"f"]) return;
	}

	[self onKeyUp:[self processKeyCode:event]];
}

- (void)scrollWheel:(NSEvent*) event
{
	gvec2 vec([event deltaX], -[event deltaY]);
	aprilWindow->handleMouseEvent(april::Window::AMOUSEEVT_SCROLL, vec, april::AK_NONE);
}

- (void)flagsChanged:(NSEvent*) event // special NSWindow function for modifier keys
{
    if ([event modifierFlags] > (1 << 15)) // DOWN
    {
		onKeyDown:april::getAprilMacKeyCode([event keyCode]);
    }
    else
    {
		onKeyUp:april::getAprilMacKeyCode([event keyCode]);
    }
}

- (void)windowDidResignKey:(NSNotification*) notification
{
	if (gReattachLoadingOverlay)
	{
		gReattachLoadingOverlay = false;
		reattachLoadingOverlay();
	}
	else aprilWindow->handleFocusChangeEvent(0);
}

- (void)windowDidBecomeKey:(NSNotification*) notification
{
	if (!gReattachLoadingOverlay)
	{
		static bool first = 0;
		if (!first) first = 1; // ignore initialization time focus gain
		else aprilWindow->handleFocusChangeEvent(1);
	}
}

- (BOOL)canBecomeKeyWindow
{
	return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)setOpenGLView:(AprilMacOpenGLView*) view
{
	mView = view;
	self.contentView = view;
}

- (void)destroy
{
	if (mTimer != nil)
	{
		[mTimer invalidate];
		mTimer = nil;
	}
}

- (void)dealloc
{
	[self destroy];
	[super dealloc];
}

@end
