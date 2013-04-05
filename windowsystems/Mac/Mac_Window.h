/// @file
/// @author  Kresimir Spes
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a MacOSX window using Apple Cocoa API.

#ifndef APRIL_MAC_WINDOW_H
#define APRIL_MAC_WINDOW_H

#include "Window.h"

namespace april
{
	class Mac_Window : public Window
	{
	public:
		Mac_Window();
		~Mac_Window();

		virtual int getWidth();
		virtual int getHeight();
		virtual void* getBackendId();
		
		bool create(int w, int h, bool fullscreen, chstr title, chstr options = "");
		bool destroy();
		
		void setTitle(chstr title);
		bool isCursorVisible();
		void setCursorVisible(bool visible);
		gvec2 getCursorPosition();
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		
        void presentFrame();
		bool updateOneFrame();
	protected:
		bool retainLoadingOverlay;
	};
	
}

#endif