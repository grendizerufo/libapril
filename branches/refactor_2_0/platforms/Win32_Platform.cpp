/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <windows.h>

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Platform.h"
#include "Window.h"

namespace april
{
	gvec2 getDisplayResolution()
	{
		return gvec2((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
	}

	SystemInfo getSystemInfo()
	{
		static SystemInfo info;
		if (info.locale == "")
		{
			info.ram = 1024;
			info.locale = "en";
		}
		return info;
	}

	DeviceType getDeviceType()
	{
		return DEVICE_WINDOWS_PC;
	}

	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		HWND hwnd = 0;
		if (april::window && (style & AMSGSTYLE_MODAL) != 0)
		{
			hwnd = (HWND)april::window->getBackendId();
		}
		int type = 0;
		if ((buttonMask & AMSGBTN_OK) != 0 && (buttonMask & AMSGBTN_CANCEL) != 0)
		{
			type |= MB_OKCANCEL;
		}
		else if ((buttonMask & AMSGBTN_YES) != 0 && (buttonMask & AMSGBTN_NO) != 0 && (buttonMask & AMSGBTN_CANCEL) != 0)
		{
			type |= MB_YESNOCANCEL;
		}
		else if ((buttonMask & AMSGBTN_OK) != 0)
		{
			type |= MB_OK;
		}
		else if ((buttonMask & AMSGBTN_YES) != 0 && (buttonMask & AMSGBTN_NO) != 0)
		{
			type |= MB_YESNO;
		}
		
		if ((style & AMSGSTYLE_INFORMATION) != 0)
		{
			type |= MB_ICONINFORMATION;
		}
		else if ((style & AMSGSTYLE_WARNING) != 0)
		{
			type |= MB_ICONWARNING;
		}
		else if ((style & AMSGSTYLE_CRITICAL) != 0)
		{
			type |= MB_ICONSTOP;
		}
		else if ((style & AMSGSTYLE_QUESTION) != 0)
		{
			type |= MB_ICONQUESTION;
		}
		
		int btn = MessageBox(hwnd, text.c_str(), title.c_str(), type);
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
	}

}
#endif
