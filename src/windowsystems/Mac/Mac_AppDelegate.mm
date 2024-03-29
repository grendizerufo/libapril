/// @file
/// @version 3.5
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Foundation/Foundation.h>
#include <hltypes/hstring.h>
#include <hltypes/hlog.h>
#import "Mac_AppDelegate.h"
#import "Mac_Window.h"
#include "april.h"

bool gAppStarted = false;

extern int gArgc;
extern char** gArgv;
extern void (*gAprilInit)(const harray<hstr>&);
extern void (*gAprilDestroy)();
NSString* getApplicationName();

bool g_WindowFocusedBeforeSleep = false;

@implementation AprilAppDelegate

- (void) receiveSleepNote: (NSNotification*) note
{
	if (aprilWindow && aprilWindow->isFocused())
	{
#ifdef _DEBUG
		hlog::write(april::logTag, "Computer went to sleep while app was focused.");
#endif
		aprilWindow->onFocusChanged(false);
		g_WindowFocusedBeforeSleep = true;
	}
	else g_WindowFocusedBeforeSleep = false;
}

- (void) receiveWakeNote: (NSNotification*) note
{
	if (g_WindowFocusedBeforeSleep)
	{
#ifdef _DEBUG
		hlog::write(april::logTag, "Computer waked from sleep, focusing window.");
#endif
		aprilWindow->onFocusChanged(true);
		g_WindowFocusedBeforeSleep = false;
	}
}

- (void) applicationDidFinishLaunching: (NSNotification*) note
{
	mAppFocused = true;

	harray<hstr> argv;
	for (int i = 0; i < gArgc; i++)
	{
		argv.add(gArgv[i]);
	}
	gAprilInit(argv);
	// register for sleep/wake notifications, needed for proper handling
	// of focus/unfocus events
	
	NSNotificationCenter* c = [[NSWorkspace sharedWorkspace] notificationCenter];
	[c addObserver:self selector: @selector(receiveSleepNote:) name:NSWorkspaceWillSleepNotification object:NULL];
	[c addObserver:self selector: @selector(receiveWakeNote:) name:NSWorkspaceDidWakeNotification object:NULL];
    
    if (april::isUsingCVDisplayLink())
    {
        hmutex::ScopeLock lock(&aprilWindow->renderThreadSyncMutex);
        gAppStarted = true;
    }
}

- (void) applicationWillTerminate:(NSNotification*) note
{
    if (april::isUsingCVDisplayLink())
    {
        hmutex::ScopeLock lock(&aprilWindow->renderThreadSyncMutex);
        gAppStarted = false;
        lock.release();
    }
	gAprilDestroy();
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
	if (mAppFocused) return; // this blocks initial app focus call
	mAppFocused = true;
#ifdef _DEBUG
	hlog::write(april::logTag, "Application activated.");
#endif
	if (aprilWindow) aprilWindow->OnAppGainedFocus();
}

- (void)applicationDidResignActive:(NSNotification *)aNotification
{
	if (!mAppFocused) return;
	mAppFocused = false;
#ifdef _DEBUG
	hlog::write(april::logTag, "Application deactivated.");
#endif
	if (aprilWindow) aprilWindow->OnAppLostFocus();
}

@end
