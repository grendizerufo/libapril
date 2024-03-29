/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a MacOSX window using Apple Cocoa API.

#ifndef APRIL_MAC_WINDOW_H
#define APRIL_MAC_WINDOW_H

#include <hltypes/hmutex.h>
#include "Window.h"
#include "Platform.h"

namespace april
{
    class QueuedEvent;
    
	class Mac_Window : public Window
	{
	public:
		Mac_Window();
		~Mac_Window();

		virtual int getWidth();
		virtual int getHeight();
		virtual void* getBackendId();
		
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();
		
		void setTitle(chstr title);
		gvec2 getCursorPosition();
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		
		void updateCursorPosition(gvec2& pos);
		bool isCursorVisible();
		void setCursor(Cursor* value);
		void setCursorVisible(bool visible);
		bool isCursorInside();

		void presentFrame();
		bool updateOneFrame();
		void terminateMainLoop();

		void setResolution(int w, int h, bool fullscreen);
		void setFullscreenFlag(bool value);

		void OnAppGainedFocus();
		void OnAppLostFocus();
		
		void onFocusChanged(bool value);
		
		bool shouldIgnoreUpdate();
		void setIgnoreUpdateFlag(bool value);
		
        void dispatchQueuedEvents();
        void queueWindowSizeChanged(int w, int h, bool fullscreen);
        void queueFocusChanged(bool focused);
        void dispatchWindowSizeChanged(int w, int h, bool fullscreen);
        void queueMessageBox(chstr title, harray<hstr> argButtons, harray<MessageBoxButton> argButtonTypes, chstr text, void (*callback)(MessageBoxButton));
        
        
		bool retainLoadingOverlay;
		bool fastHideLoadingOverlay;
		bool ignoreUpdate;
		hmutex ignoreUpdateMutex;
		bool splashScreenFadeout;
		bool fpsCounter;
		hstr fpsTitle;

		float scalingFactor;
        hmutex renderThreadSyncMutex;
	protected:
		Cursor* _createCursor();
        harray<QueuedEvent*> queuedEvents;
	};
	
    bool isUsingCVDisplayLink();
}
extern april::Mac_Window* aprilWindow;

bool isPreLion();
bool isLionOrNewer();

#endif
