/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENKODE_WINDOW
#ifdef __APPLE__
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE
		#import <UIKit/UIWindow.h>
	#endif
#endif

#include <KD/kd.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"

#ifdef _EGL
#include "egl.h"
#endif
#include "OpenGL_RenderSystem.h"
#include "OpenKODE_Window.h"
#include "OpenKODE_Keys.h"

#if TARGET_OS_IPHONE
#import <AVFoundation/AVFoundation.h>

bool (*iOShandleUrlCallback)(chstr url) = NULL; // KD-TODO
#endif

namespace april
{
#if TARGET_OS_IPHONE
	static void iosSetupAudioSession()
	{
		// kspes: copied this from iOS app delegate code, it's needed for OpenKODE and OpenAL to play along on iOS
		if ([[[UIDevice currentDevice] systemVersion] compare:@"5.0" options:NSNumericSearch] == NSOrderedAscending)
		{
			// less then iOS 5.0 - workarround for an apple bug where the audio sesion get's interrupted while using AVAssetReader and similar
			AVAudioSession *audioSession = [AVAudioSession sharedInstance];
			[audioSession setActive: NO error: nil];
			[audioSession setCategory:AVAudioSessionCategoryPlayback error:nil];
			
			// Modifying Playback Mixing Behavior, allow playing music in other apps
			UInt32 allowMixing = true;
			AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryMixWithOthers, sizeof(allowMixing), &allowMixing);
			[audioSession setActive: YES error: nil];
		}
		else
		{
			[[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryAmbient error:NULL];
		}
	}
#endif
	
	void KD_APIENTRY lowMemoryEventCallback(const KDEvent * _event)
	{
		switch (_event->type)
		{
			case KD_EVENT_LOWMEM:
			{
				hlog::write(logTag, "Received libKD memory warning!");
				if (april::window) april::window->handleLowMemoryWarning();
				break;
			}
		}
	}
	
	OpenKODE_Window::OpenKODE_Window() : Window()
	{
#if TARGET_OS_IPHONE
		iosSetupAudioSession();
#endif
		this->name = APRIL_WS_OPENKODE;
		this->kdWindow = NULL;
		this->virtualKeyboardVisible = false;
		memset(this->kdTouches, 0, 16 * sizeof(bool));
#if defined(_WIN32) && !defined(_EGL)
		hlog::warn(april::logTag, "OpenKODE Window requires EGL to be present!");
#endif
		initOpenKODEKeyMap();
	}

	OpenKODE_Window::~OpenKODE_Window()
	{
		this->destroy();
	}

	bool OpenKODE_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->virtualKeyboardVisible = false;
		if (w <= 0 || h <= 0)
		{
			hlog::errorf(april::logTag, "Cannot create window with size: %d x %d", w, h);
			this->destroy();
			return false;
		}
#ifdef _EGL
		this->kdWindow = kdCreateWindow(april::egl->display, april::egl->config, NULL);
#endif
		if (this->kdWindow == NULL)
		{
			hlog::error(april::logTag, "Can't create KD Window!");
			this->destroy();
			return false;
		}
		if (fullscreen) // KD only supports fullscreen in the screen's resolution
		{
			kdQueryAttribi(KD_ATTRIB_WIDTH, &w);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, &h);
		}
		KDint32 size[] = {w, h};
		kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		kdSetWindowPropertycv(this->kdWindow, KD_WINDOWPROPERTY_CAPTION, title.c_str());
#ifdef _EGL // KD doesn't actually work without EGL
		if (kdRealizeWindow(this->kdWindow, &april::egl->hWnd) != 0)
#endif
		{
			hlog::error(april::logTag, "Can't realize KD Window!");
			this->destroy();
			return false;
		}
#ifdef _EGL
		april::egl->create();
#endif
		
		kdInstallCallback(&lowMemoryEventCallback, KD_EVENT_LOWMEM, NULL);
		return true;
	}
	
	bool OpenKODE_Window::destroy()
	{
		kdInstallCallback(NULL, KD_EVENT_LOWMEM, NULL);

		if (!Window::destroy())
		{
			return false;
		}
		this->virtualKeyboardVisible = false;
#ifdef _EGL
		april::egl->destroy();
#endif
		if (this->kdWindow != NULL)
		{
			kdDestroyWindow(this->kdWindow);
			this->kdWindow = NULL;
		}
		return true;
	}

	int OpenKODE_Window::getWidth()
	{
		KDint32 size[] = {0, 0};
		kdGetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		return size[0];
	}
	
	int OpenKODE_Window::getHeight()
	{
		KDint32 size[] = {0, 0};
		kdGetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		return size[1];
	}
	
	void OpenKODE_Window::setTitle(chstr title)
	{
		this->title = title;
		kdSetWindowPropertycv(this->kdWindow, KD_WINDOWPROPERTY_CAPTION, this->title.c_str());
	}
	
	bool OpenKODE_Window::isCursorVisible()
	{
		return (Window::isCursorVisible() || !this->isCursorInside());
	}
	
	void OpenKODE_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
	}
	
	void* OpenKODE_Window::getBackendId()
	{
#if TARGET_OS_IPHONE
		UIWindow* window = [UIApplication sharedApplication].keyWindow;
		if (!window)
			window = [[UIApplication sharedApplication].windows objectAtIndex:0];
		
		UIViewController* controller = [window rootViewController];

		return controller;
#elif defined(_EGL)
		return april::egl->hWnd;
#else
		return 0;
#endif
	}

	void OpenKODE_Window::setResolution(int w, int h, bool fullscreen)
	{
		if (fullscreen) // KD only supports fullscreen in the screen's resolution
		{
			kdQueryAttribi(KD_ATTRIB_WIDTH, &w);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, &h);
		}
		this->fullscreen = fullscreen;
		KDint32 size[] = {w, h};
		kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		
		april::SystemDelegate* delegate = this->getSystemDelegate();
		april::rendersys->setViewport(grect(0, 0, w, h));

		if (delegate)
		{
			delegate->onWindowSizeChanged(w, h, fullscreen);
		}

	}

	void OpenKODE_Window::handleActivityChangeEvent(bool active)
	{
		if (active != this->focused)
		{
			this->handleFocusChangeEvent(active);
		}
	}

	bool OpenKODE_Window::updateOneFrame()
	{
		static bool result;
		this->checkEvents();
		// rendering
		result = Window::updateOneFrame();
		this->setTitle(this->title); // has to come after Window::updateOneFrame(), otherwise FPS value in title would be late one frame
		return result;
	}
	
	void OpenKODE_Window::presentFrame()
	{
#ifdef _EGL
		april::egl->swapBuffers();
#endif
	}
	
	int OpenKODE_Window::getAprilTouchIndex(int kdIndex)
	{
		int index = 0;
		for (int i = 0; i <= kdIndex; i++)
		{
			if (this->kdTouches[i])
			{
				index++;
			}
		}
		
		return index -1;
	}

	bool OpenKODE_Window::_processEvent(const KDEvent* evt)
	{
		switch (evt->type)
		{
		case KD_EVENT_QUIT:
			this->handleQuitRequest(false);
			this->terminateMainLoop();
			return true;
		case KD_EVENT_WINDOW_CLOSE:
			this->handleQuitRequest(true);
			return true;
		case KD_EVENT_PAUSE:
			hlog::write(logTag, "OpenKODE pause event received.");
			this->handleActivityChangeEvent(false);
			april::rendersys->unloadTextures();
#ifdef _EGL
			april::egl->destroy();
#endif
			return true;
		case KD_EVENT_RESUME:
#ifdef _EGL
			april::egl->create();
#endif
			hlog::write(logTag, "OpenKODE resume event received.");
			this->handleActivityChangeEvent(true);
			return true;
		case KD_EVENT_WINDOW_FOCUS:
			{
				bool active = evt->data.windowfocus.focusstate != 0;
				if (this->focused != active)
					this->handleFocusChangeEvent(active);
			}
			return true;
		case KD_EVENT_INPUT:
			if (evt->data.input.value.i != 0)
			{
				if (evt->data.input.index < KD_IOGROUP_CHARS) // because key and char events are separate
				{
					this->queueKeyEvent(april::Window::AKEYEVT_DOWN, kd2april(evt->data.input.index), 0);
				}
				else
				{
					this->queueKeyEvent(april::Window::AKEYEVT_DOWN, april::AK_NONE, evt->data.input.index - KD_IOGROUP_CHARS);
				}
			}
			else
			{
				this->queueKeyEvent(april::Window::AKEYEVT_UP, kd2april(evt->data.input.index), 0);
			}
			return true;
		case KD_EVENT_INPUT_POINTER:
			{
#ifdef _PC_INPUT
				int index = evt->data.inputpointer.index;
				gvec2 pos((float)evt->data.inputpointer.x, (float)evt->data.inputpointer.y);

				if (index == KD_INPUT_POINTER_X || index == KD_INPUT_POINTER_Y)
				{
					this->queueMouseEvent(Window::AMOUSEEVT_MOVE, pos, AK_NONE);
				}
				else if (index == KD_INPUT_POINTER_SELECT)
				{
					int s = evt->data.inputpointer.select;
					bool state[3];
					state[0] = s & 1;
					state[1] = s & 2;
					state[2] = s & 4;
					
					for (int i = 0; i < 3; i++)
					{
						if (state[i] != this->kdTouches[i])
						{							
							this->queueMouseEvent(state[i] ? Window::AMOUSEEVT_DOWN : Window::AMOUSEEVT_UP, pos, (i == 0 ? AK_LBUTTON : (i == 1 ? AK_RBUTTON : AK_MBUTTON)));
						}
					}
					memcpy(this->kdTouches, state, 3 * sizeof(bool));
				}
				this->cursorPosition = pos;
#else
				int i, j, index = evt->data.inputpointer.index;
				gvec2 pos((float)evt->data.inputpointer.x, (float)evt->data.inputpointer.y);
				for (i = 0, j = 0; i < 4; i++, j += KD_IO_POINTER_STRIDE)
				{
					if (index == KD_INPUT_POINTER_X + j || index == KD_INPUT_POINTER_Y + j)
					{
						this->queueTouchEvent(Window::AMOUSEEVT_MOVE, pos, this->getAprilTouchIndex(i));
					}
					else if (index == KD_INPUT_POINTER_SELECT + j)
					{
						if (evt->data.inputpointer.select != 0)
						{
							this->kdTouches[i] = true;
							this->queueTouchEvent(Window::AMOUSEEVT_DOWN, pos, this->getAprilTouchIndex(i));
						}
						else
						{
							this->queueTouchEvent(Window::AMOUSEEVT_UP, pos, this->getAprilTouchIndex(i));
							this->kdTouches[i] = false;
						}
					}
					else
						continue;
					break;
				}
				if (i == 0)
				{
					this->cursorPosition = pos;
				}
#endif
			}
			return true;
		case KD_EVENT_WINDOWPROPERTY_CHANGE:
			if (evt->data.windowproperty.pname == KD_WINDOWPROPERTY_SIZE)
			{
				this->_setRenderSystemResolution();
			}
			return true;
		}
		return false;
	}
	
	void OpenKODE_Window::checkEvents()
	{
		kdPumpEvents();
		const KDEvent* evt;
		// 1 milisecond as timeout
		while (this->running && (evt = kdWaitEvent(1000000L)) != NULL)
		{
			if (!this->_processEvent(evt))
			{
				kdDefaultEvent(evt);
			}
		}
		Window::checkEvents();
	}

	void OpenKODE_Window::beginKeyboardHandling()
	{
		kdKeyboardShow(this->kdWindow, 1);
		this->virtualKeyboardVisible = true;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(true);
		}
	}

	void OpenKODE_Window::terminateKeyboardHandling()
	{
		kdKeyboardShow(this->kdWindow, 0);
		this->virtualKeyboardVisible = false;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(false);
		}
	}

}
#endif