/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WIN32) && !defined(_OPENKODE) && !defined(_WINRT)
#include <gtypes/Vector2.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include <Psapi.h> // has to be here after hplatform.h that includes windows.h

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	extern SystemInfo info;
	
	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			info.name = "Windows";
			OSVERSIONINFO osinfo;
			memset(&osinfo, 0, sizeof(osinfo));
			osinfo.dwOSVersionInfoSize = sizeof(osinfo);
			GetVersionEx(&osinfo);
			info.osVersion = hsprintf("%d.%d", osinfo.dwMajorVersion, osinfo.dwMinorVersion);
			if      (osinfo.dwMajorVersion == 5) info.name += " XP";
			else if (osinfo.dwMajorVersion == 6)
			{
				if      (osinfo.dwMinorVersion == 0) info.name += " Vista";
				else if (osinfo.dwMinorVersion == 1) info.name += " 7";
				else if (osinfo.dwMinorVersion == 2) info.name += " 8";
				else if (osinfo.dwMinorVersion == 3) info.name += " 8.1";
				// future and special versions of Windows will just be named "Windows" to avoid assumptions
			}
			info.architecture = "x86";
			// number of CPU cores
			SYSTEM_INFO w32info;
			GetNativeSystemInfo(&w32info);
			info.cpuCores = w32info.dwNumberOfProcessors;
			// RAM size
			MEMORYSTATUSEX status;
			status.dwLength = sizeof(status);
			GlobalMemoryStatusEx(&status);
			info.ram = (int)(status.ullTotalPhys / 1048576);
			// display resolution
			info.displayResolution.set((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
			// display DPI
			info.displayDpi = 96.0f;
			// other
			info.locale = "en"; // default is "en"
			wchar_t locale[LOCALE_NAME_MAX_LENGTH] = { 0 };
			int length = GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME,
				locale, (LOCALE_NAME_MAX_LENGTH - 1) * sizeof(wchar_t));
			if (length > 0)
			{
				info.locale = hstr::fromUnicode(locale);
			}
			length = GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME,
				locale, (LOCALE_NAME_MAX_LENGTH - 1) * sizeof(wchar_t));
			if (length > 0)
			{
				info.localeVariant = hstr::fromUnicode(locale);
			}
			info.locale = info.locale.lowered();
			info.localeVariant = info.localeVariant.uppered();
		}
		return info;
	}

	hstr getPackageName()
	{
		hlog::warn(logTag, "Cannot use getPackageName() on this platform.");
		return "";
	}

	hstr getUserDataPath()
	{
		return henv("APPDATA");
	}
	
	int64_t getRamConsumption()
	{
		int64_t result = 0LL;
		PROCESS_MEMORY_COUNTERS counters;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
		{
			result = (int64_t)counters.WorkingSetSize;
		}
		return result;
	}	
	
	static void(*currentCallback)(MessageBoxButton) = NULL;

	void _messageBoxResult(int button)
	{
		switch (button)
		{
		case IDOK:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MESSAGE_BUTTON_OK);
			}
			break;
		case IDYES:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MESSAGE_BUTTON_YES);
			}
			break;
		case IDNO:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MESSAGE_BUTTON_NO);
			}
			break;
		case IDCANCEL:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MESSAGE_BUTTON_CANCEL);
			}
			break;
		default:
			hlog::error(logTag, "Unknown message box callback: " + hstr(button));
			break;
		}
	}

	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		currentCallback = callback;
		int type = 0;
		if ((buttonMask & MESSAGE_BUTTON_OK) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			type |= MB_OKCANCEL;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			type |= MB_YESNOCANCEL;
		}
		else if (buttonMask & MESSAGE_BUTTON_OK)
		{
			type |= MB_OK;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
		{
			type |= MB_YESNO;
		}
		
		if (style & MESSAGE_STYLE_INFO)
		{
			type |= MB_ICONINFORMATION;
		}
		else if (style & MESSAGE_STYLE_WARNING)
		{
			type |= MB_ICONWARNING;
		}
		else if (style & MESSAGE_STYLE_CRITICAL)
		{
			type |= MB_ICONSTOP;
		}
		else if (style & MESSAGE_STYLE_QUESTION)
		{
			type |= MB_ICONQUESTION;
		}

		HWND hwnd = 0;
		if (april::window != NULL && (style & MESSAGE_STYLE_MODAL))
		{
			hwnd = (HWND)april::window->getBackendId();
		}
		int button = MessageBoxW(hwnd, text.wStr().c_str(), title.wStr().c_str(), type);
		_messageBoxResult(button);
	}

}
#endif
