/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#if !defined(__APPLE__) || defined(__APPLE__) && (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
	#ifdef HAVE_SDL
		#include "SDLWindow.h"
	#endif
	#ifdef _WIN32
		#include "Win32Window.h"
	#endif
	#ifdef _ANDROID
		#include "AndroidJNIWindow.h"
	#endif
#elif (TARGET_OS_IPHONE)
	#import <UIKit/UIKit.h>
	#include "iOSWindow.h"
#endif

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
	#import <Foundation/NSString.h>
	#import <AppKit/NSScreen.h>
	#import <AppKit/NSPanel.h>

	#import <AppKit/NSApplication.h>
	#import <AppKit/NSWindow.h>
	#import <AppKit/NSEvent.h>
	#import <AppKit/NSCursor.h>
#endif

#include "Window.h"
#include "Keys.h"
#include "RenderSystem.h"


#if TARGET_OS_IPHONE
@interface AprilMessageBoxDelegate : NSObject<UIAlertViewDelegate> {
    void(*callback)(april::MessageBoxButton);
    april::MessageBoxButton buttonTypes[3];
	
	CFRunLoopRef runLoop;
	BOOL isModal;
	april::MessageBoxButton selectedButton;
}
@property (nonatomic, assign) void(*callback)(april::MessageBoxButton);
@property (nonatomic, assign) april::MessageBoxButton *buttonTypes;
@property (nonatomic, readonly) april::MessageBoxButton selectedButton;
@end
@implementation AprilMessageBoxDelegate
@synthesize callback;
@synthesize selectedButton;
@dynamic buttonTypes;
-(id)initWithModality:(BOOL)_isModal
{
	self = [super init];
	if(self)
	{
		runLoop = CFRunLoopGetCurrent();
		isModal = _isModal;
	}
	return self;
}
-(april::MessageBoxButton*)buttonTypes
{
    return buttonTypes;
}
-(void)setButtonTypes:(april::MessageBoxButton*)_buttonTypes
{
    memcpy(buttonTypes, _buttonTypes, sizeof(april::MessageBoxButton)*3);
}
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (callback)
    {
        callback(buttonTypes[buttonIndex]);
    }
	if (isModal)
	{
		CFRunLoopStop(runLoop);
	}
	
	selectedButton = buttonTypes[buttonIndex];
	
	[self release];
}
- (void)willPresentAlertView:(UIAlertView*)alertView
{
	
	NSString *reqSysVer = @"4.0";
	NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
	BOOL isFourOh = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
	
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone && buttonTypes[2] && isFourOh) 
	{
		// landscape sucks on 4.0+ phones when we have three buttons.
		// it doesnt show hint message.
		// unless we hack.
		
		float w = alertView.bounds.size.width;
		if(w < 5.)
		{
			april::log("In messageBox()'s label hack, width override took place");
			w = 400; // hardcoded width! seems to work ok
			
		}
		
		
		UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 30.0f, alertView.bounds.size.width, 40.0f)]; 
		label.backgroundColor = [UIColor clearColor]; 
		label.textColor = [UIColor whiteColor]; 
		label.font = [UIFont systemFontOfSize:14.0f]; 
		label.textAlignment = UITextAlignmentCenter;
		label.text = alertView.message; 
		[alertView addSubview:label]; 
		[label release];
	}
	
}
@end

#endif

namespace april
{
	Window* Window::mSingleton = NULL;

	Window::Window()
	{
		mUpdateCallback = NULL;
		mMouseDownCallback = NULL;
		mMouseUpCallback = NULL;
		mMouseMoveCallback = NULL;
		mKeyDownCallback = NULL;
		mKeyUpCallback = NULL;
		mCharCallback = NULL;
		mQuitCallback = NULL;
		mFocusCallback = NULL;
		mVKeyboardCallback = NULL;
		mDeviceOrientationCallback = NULL;
		mTouchEnabledCallback = NULL;
		mTouchCallback = NULL;
		mLowMemoryCallback = NULL;
		mHandleURLCallback = NULL;
		mSingleton = this;
	}
	
	Window::~Window()
	{
	
	}

	void Window::setUpdateCallback(bool (*callback)(float))
	{
		mUpdateCallback = callback;
	}

	void Window::setMouseCallbacks(void (*mouse_dn)(float, float, int),
								   void (*mouse_up)(float, float, int),
								   void (*mouse_move)(float, float))
	{
		mMouseDownCallback = mouse_dn;
		mMouseUpCallback = mouse_up;
		mMouseMoveCallback = mouse_move;
	}
	
	void Window::setKeyboardCallbacks(void (*key_dn)(unsigned int),
									  void (*key_up)(unsigned int),
									  void (*char_callback)(unsigned int))
	{
		mKeyDownCallback = key_dn;
		mKeyUpCallback = key_up;
		mCharCallback = char_callback;
	}
	
	void Window::setLowMemoryCallback(void (*lowmem_callback)())
	{
		mLowMemoryCallback=lowmem_callback;
	}
	
	void Window::setHandleURLCallback(bool (*url_callback)(chstr))
	{
		mHandleURLCallback = url_callback;
	}
	
	void Window::handleLowMemoryWarning()
	{
		if (mLowMemoryCallback) mLowMemoryCallback();
	}
	
	void Window::setQuitCallback(bool (*quit_callback)(bool))
	{
		mQuitCallback = quit_callback;
	}
	
	void Window::setWindowFocusCallback(void (*focus_callback)(bool))
	{
		mFocusCallback = focus_callback;
	}
	
	void Window::setVirtualKeyboardCallback(void (*vk_callback)(bool))
	{
		mVKeyboardCallback = vk_callback;
	}
	
	void Window::setTouchscreenEnabledCallback(void (*te_callback)(bool))
	{
		mTouchEnabledCallback = te_callback;
	}
	
	void Window::setTouchEventCallback(void (*t_callback)(harray<gvec2>&))
	{
		mTouchCallback = t_callback;
	}
	
	void Window::setDeviceOrientationCallback(void (*do_callback)(DeviceOrientation))
	{
		mDeviceOrientationCallback = do_callback;
	}
	
	bool Window::isFullscreen()
	{
		return mFullscreen;
	}
	
	float Window::getAspectRatio()
	{
		return ((float)getWidth() / getHeight());
	}
	
	bool Window::performUpdate(float k)
	{
		// returning true: continue execution
		// returning false: abort execution
		if (mUpdateCallback != NULL)
		{
			return (*mUpdateCallback)(k);
		}
		return true;
	}
	
	void Window::handleKeyEvent(KeyEventType type, KeySym keycode, unsigned int unicode)
	{
		if (keycode == AK_UNKNOWN)
		{
			april::log("key event on unknown key");
			keycode = AK_NONE;
		}
		switch (type)
		{
		case AKEYEVT_DOWN:
			if (mKeyDownCallback != NULL && keycode != AK_NONE)
			{
				(*mKeyDownCallback)(keycode);
			}
			if (unicode != 0 && mCharCallback != NULL && unicode != 127) // hack for sdl on mac, backspace induces character
			{
				(*mCharCallback)(unicode);
			}
			break;
		case AKEYEVT_UP:
			if (mKeyUpCallback != NULL && keycode != AK_NONE)
			{
				(*mKeyUpCallback)(keycode);
			}
			break;
		default:
			break;
		}
	}
	
	void Window::handleMouseEvent(MouseEventType event, float x, float y, MouseButton button)
	{
		switch (event)
		{
		case AMOUSEEVT_DOWN:
			if (mMouseDownCallback != NULL)
			{
				(*mMouseDownCallback)(x, y, button);
			}
			break;
		case AMOUSEEVT_UP:
			if (mMouseUpCallback != NULL)
			{
				(*mMouseUpCallback)(x, y, button);
			}
			break;
		case AMOUSEEVT_MOVE:
			if (mMouseMoveCallback != NULL)
			{
				(*mMouseMoveCallback)(x, y);
			}
			break;
		}
	}
	
	void Window::handleTouchEvent(harray<gvec2>& touches)
	{
		if (mTouchCallback != NULL)
		{
			(*mTouchCallback)(touches);
		}
	}

	gvec2 Window::getDimensions()
	{
		return gvec2((float)getWidth(), (float)getHeight());
	}
	
	bool Window::isCursorInside()
	{
		gvec2 v = getCursorPosition();
		return (is_in_range(v.x, 0.0f, (float)getWidth()) && is_in_range(v.y, 0.0f, (float)getHeight()));
	}
	
	bool Window::handleQuitRequest(bool can_reject)
	{
		// returns whether or not the windowing system is permitted to close the window
		if (mQuitCallback != NULL)
		{
			return (*mQuitCallback)(can_reject);
		}
		return true;
	}
	
	void Window::handleFocusEvent(bool has_focus)
	{
		if (mFocusCallback != NULL)
		{
			(*mFocusCallback)(has_focus);
		}
	}
	
	bool Window::handleURL(chstr url)
	{
		return (mHandleURLCallback != NULL && (*mHandleURLCallback)(url));
	}
	
	void Window::beginKeyboardHandling()
	{
		// ignore by default
		// used only for softkeyboards, e.g. iOS
	}
	
	void Window::terminateKeyboardHandling()
	{
		// ignore by default
		// used only for softkeyboards, e.g. iOS
	}
	
	float Window::prefixRotationAngle()
	{
		// some platforms such as iOS may need extra 
		// rotation before they can render anything
		// with certain orientations.
		
		// earlier versions of iOS always launch with
		// portrait orientation and require us to
		// manually rotate
		return 0.0f;
	}

	
	void Window::_platformCursorVisibilityUpdate(bool visible)
	{
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		NSWindow* window = [[NSApplication sharedApplication] keyWindow];
		bool shouldShow;
		
		if (!visible)
		{
			//NSPoint 	mouseLoc = [window convertScreenToBase:[NSEvent mouseLocation]];
			//[window frame]
			NSPoint mouseLoc;
			id hideInsideView; // either NSView or NSWindow; both implement "frame" method
			if ([window contentView])
			{
				hideInsideView = [window contentView];
				mouseLoc = [window convertScreenToBase:[NSEvent mouseLocation]];
			}
			else
			{
				hideInsideView = window;
				mouseLoc = [NSEvent mouseLocation];
			}
			
			if (hideInsideView)
			{
				shouldShow = !NSPointInRect(mouseLoc, [hideInsideView frame]);
			}
			else // no view? let's presume we are in fullscreen where we should blindly honor the requests from the game
			{
				shouldShow = false;
			}
		}
		else
		{			
			shouldShow = true;
		}
		
		if (!shouldShow && CGCursorIsVisible())
		{
			CGDisplayHideCursor(kCGDirectMainDisplay);
		}
		else if (shouldShow && !CGCursorIsVisible())
		{
			CGDisplayShowCursor(kCGDirectMainDisplay);
		}
#endif
	}
	
	///////////////////////
	// non members
	///////////////////////
	
	Window* createAprilWindow(chstr winsysname, int w, int h, bool fullscreen, chstr title)
	{
#if !defined(__APPLE__) || defined(__APPLE__) && (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
		// desktop
	#ifdef HAVE_SDL
		if (winsysname == "SDL")
		{
			return new SDLWindow(w, h, fullscreen, title);
		}
	#endif
	#ifdef _WIN32
		if (winsysname == "Win32")
		{
			return new Win32Window(w, h, fullscreen, title);
		}
	#endif
	#ifdef _ANDROID
		if (winsysname == "AndroidJNI")
		{
			return new AndroidJNIWindow(w, h, fullscreen, title);
		}
	#endif
		
#elif (TARGET_OS_IPHONE)
		// iOS
		return Window::getSingleton();
#endif
		return NULL;
	}
	
	gvec2 getDesktopResolution()
	{
#ifdef _WIN32
		return gvec2((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
#elif (TARGET_OS_IPHONE)
		UIScreen* mainScreen = [UIScreen mainScreen];
		float scale = 1;
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		if ([mainScreen respondsToSelector:@selector(scale)])
		{
			scale = [mainScreen scale];
		}
#endif
		// flipped width and height, to get a 4:3 ratio instead of 3:4
		int w = mainScreen.bounds.size.height * scale;
		int h = mainScreen.bounds.size.width * scale;
		return gvec2(w, h);
#elif (TARGET_OS_MAC)
		NSScreen* mainScreen = [NSScreen mainScreen];
		NSRect rect = [mainScreen frame];
		return gvec2(rect.size.width, rect.size.height);
#else
		return gvec2(1024, 768);
#endif
	}
	
	static MessageBoxButton messageBox_impl(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
#ifdef _WIN32
		HWND wnd = 0;
		if (rendersys && rendersys->getWindow() && style & AMSGSTYLE_MODAL)
		{
			wnd = (HWND)rendersys->getWindow()->getIDFromBackend();
		}
		int type = 0;
		if (buttonMask & AMSGBTN_OK && buttonMask & AMSGBTN_CANCEL)
		{
			type |= MB_OKCANCEL;
		}
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO && buttonMask & AMSGBTN_CANCEL)
		{
			type |= MB_YESNOCANCEL;
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			type |= MB_OK;
		}
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO)
		{
			type |= MB_YESNO;
		}
		
		if (style & AMSGSTYLE_INFORMATION)
		{
			type |= MB_ICONINFORMATION;
		}
		else if (style & AMSGSTYLE_WARNING)
		{
			type |= MB_ICONWARNING;
		}
		else if (style & AMSGSTYLE_CRITICAL)
		{
			type |= MB_ICONSTOP;
		}
		else if (style & AMSGSTYLE_QUESTION)
		{
			type |= MB_ICONQUESTION;
		}
		
		int btn = MessageBox(wnd, text.c_str(), title.c_str(), type);
		switch(btn)
		{
		case IDOK:
            if (callback != NULL)
			{
                (*callback)(AMSGBTN_OK);
			}
			return AMSGBTN_OK;
		case IDYES:
            if (callback != NULL)
			{
                (*callback)(AMSGBTN_YES);
			}
			return AMSGBTN_YES;
		case IDNO:
            if (callback != NULL)
			{
                (*callback)(AMSGBTN_NO);
			}
			return AMSGBTN_NO;
		case IDCANCEL:
            if (callback != NULL)
			{
                (*callback)(AMSGBTN_CANCEL);
			}
			return AMSGBTN_CANCEL;
		}
		return AMSGBTN_OK;
#elif TARGET_OS_MAC && !TARGET_OS_IPHONE
		// fugly implementation of showing messagebox on mac os
		// ideas:
		// * display as a sheet attached on top of the current window
		// * prioritize buttons and automatically assign slots
		// * use constants for button captions
		// * use an array with constants for button captions etc
		
		NSString *buttons[] = {@"Ok", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[] = {AMSGBTN_OK, AMSGBTN_NULL, AMSGBTN_NULL};
        
		if (buttonMask & AMSGBTN_OK && buttonMask & AMSGBTN_CANCEL)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_OK, "Ok").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            
            buttonTypes[0] = AMSGBTN_OK;
            buttonTypes[1] = AMSGBTN_CANCEL;
		}
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO && buttonMask & AMSGBTN_CANCEL)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
			buttons[2] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            
            buttonTypes[0] = AMSGBTN_YES;
            buttonTypes[1] = AMSGBTN_NO;
            buttonTypes[2] = AMSGBTN_CANCEL;

		}
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
            
            buttonTypes[0] = AMSGBTN_YES;
            buttonTypes[1] = AMSGBTN_NO;
		}
		else if (buttonMask & AMSGBTN_CANCEL)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            buttonTypes[0] = AMSGBTN_CANCEL;
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_OK, "Ok").c_str()];
            buttonTypes[0] = AMSGBTN_OK;
		}
		else if (buttonMask & AMSGBTN_YES)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
            buttonTypes[0] = AMSGBTN_YES;
		}
		else if (buttonMask & AMSGBTN_NO)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
            buttonTypes[0] = AMSGBTN_NO;
		}
		
		NSString *titlens = [NSString stringWithUTF8String:title.c_str()];
		NSString *textns = [NSString stringWithUTF8String:text.c_str()];
		
		int clicked = NSRunAlertPanel(titlens, textns, buttons[0], buttons[1], buttons[2]);
		switch (clicked)
		{
		case NSAlertDefaultReturn:
			clicked = 0;
			break;
		case NSAlertAlternateReturn:
			clicked = 1;
			break;
		case NSAlertOtherReturn:
			clicked = 2;
			break;
		}
        
        if (callback != NULL)
        {
            (*callback)(buttonTypes[clicked]);
        }
        return buttonTypes[clicked];
        
#elif TARGET_OS_IPHONE

        NSString *buttons[] = {@"Ok", nil, nil}; // set all buttons to nil, at first, except default one, just in case
		MessageBoxButton buttonTypes[] = {AMSGBTN_OK, AMSGBTN_NULL, AMSGBTN_NULL};
        
		if (buttonMask & AMSGBTN_OK && buttonMask & AMSGBTN_CANCEL)
		{
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_OK, "Ok").c_str()];
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            
            buttonTypes[1] = AMSGBTN_OK;
            buttonTypes[0] = AMSGBTN_CANCEL;
        }
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO && buttonMask & AMSGBTN_CANCEL)
		{
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
			buttons[2] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            
            buttonTypes[1] = AMSGBTN_YES;
            buttonTypes[2] = AMSGBTN_NO;
            buttonTypes[0] = AMSGBTN_CANCEL;
		}
		else if (buttonMask & AMSGBTN_YES && buttonMask & AMSGBTN_NO)
		{
			buttons[1] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
            
            buttonTypes[1] = AMSGBTN_YES;
            buttonTypes[0] = AMSGBTN_NO;
		}
		else if (buttonMask & AMSGBTN_CANCEL)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel").c_str()];
            buttonTypes[0] = AMSGBTN_CANCEL;
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_OK, "Ok").c_str()];
            buttonTypes[0] = AMSGBTN_OK;
		}
		else if (buttonMask & AMSGBTN_YES)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes").c_str()];
            buttonTypes[0] = AMSGBTN_YES;
		}
		else if (buttonMask & AMSGBTN_NO)
		{
			buttons[0] = [NSString stringWithUTF8String:customButtonTitles.try_get_by_key(AMSGBTN_NO, "No").c_str()];
            buttonTypes[0] = AMSGBTN_NO;
		}
		
		NSString *titlens = [NSString stringWithUTF8String:title.c_str()];
		NSString *textns = [NSString stringWithUTF8String:text.c_str()];

        AprilMessageBoxDelegate *mbd = [[[AprilMessageBoxDelegate alloc] initWithModality:(style & AMSGSTYLE_MODAL)] autorelease];
        mbd.callback = callback;
        mbd.buttonTypes = buttonTypes;
		[mbd retain];

		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:titlens
														message:textns
													   delegate:mbd 
											  cancelButtonTitle:buttons[0]
											  otherButtonTitles:buttons[1], buttons[2], nil];
		[alert show];
		if (style & AMSGSTYLE_MODAL) 
		{
			CFRunLoopRun();
		}
		[alert release];
		
		// We're modal?
		// If so, we know what to return!
		if (style & AMSGSTYLE_MODAL)
		{
			return mbd.selectedButton;
		}
		
		// NOTE: does not return proper values unless modal! 
		//       you need to implement a delegate.
		
		
		// some dummy returnvalues
		if (buttonMask & AMSGBTN_CANCEL)
		{
			return AMSGBTN_CANCEL;
		}
		if (buttonMask & AMSGBTN_OK)
		{
			return AMSGBTN_OK;
		}
		if (buttonMask & AMSGBTN_NO)
		{
			return AMSGBTN_NO;
		}
		if (buttonMask & AMSGBTN_YES)
		{
			return AMSGBTN_YES;
		}
		return AMSGBTN_OK;
//#elif defined(HAVE_MARMLADE)
// TODO
#else
		april::log(hsprintf("== %s ==", title.c_str()));
		april::log(text);
		april::log(hsprintf("Button mask: %c%c%c%c", 
							   buttonMask & AMSGBTN_OK ? '+' : '-', 
							   buttonMask & AMSGBTN_CANCEL ? '+' : '-',
							   buttonMask & AMSGBTN_YES ? '+' : '-',
							   buttonMask & AMSGBTN_NO ? '+' : '-'));
                   
		// some dummy returnvalues
		if (buttonMask & AMSGBTN_CANCEL)
		{
            if (callback != NULL)
			{
                (*callback)(AMSGBTN_CANCEL);
			}
			return AMSGBTN_CANCEL;
		}
		if (buttonMask & AMSGBTN_OK)
		{
            if (callback != NULL)
			{
                (*callback)(AMSGBTN_OK);
			}
			return AMSGBTN_OK;
		}
		if (buttonMask & AMSGBTN_NO)
		{
            if (callback != NULL)
			{
                (*callback)(AMSGBTN_NO);
			}
			return AMSGBTN_NO;
		}
		if (buttonMask & AMSGBTN_YES)
		{
            if (callback != NULL)
			{
                (*callback)(AMSGBTN_YES);
			}
			return AMSGBTN_YES;
		}
                   
        if (callback != NULL)
		{
            (*callback)(AMSGBTN_OK);
		}
		return AMSGBTN_OK;
#endif
	}

	MessageBoxButton messageBox(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		MessageBoxStyle passedStyle = style;
		if (style & AMSGSTYLE_TERMINATEAPPONDISPLAY) 
		{
#if !TARGET_OS_IPHONE
			rendersys->getWindow()->terminateMainLoop();
			rendersys->getWindow()->destroyWindow();
#endif
			passedStyle = (MessageBoxStyle)(passedStyle & AMSGSTYLE_MODAL);
		}
		MessageBoxButton returnValue = messageBox_impl(title, text, buttonMask, passedStyle, customButtonTitles, callback);
		if (style & AMSGSTYLE_TERMINATEAPPONDISPLAY)
		{
			exit(1);
		}
		return returnValue;
	}
	
	
}
