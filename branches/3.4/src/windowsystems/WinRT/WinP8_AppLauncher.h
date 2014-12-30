/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinRT Phone 8 App Launcher.

#if defined(_WINRT_WINDOW) && defined(_WINP8)
#ifndef APRIL_WINP8_APP_LAUNCHER_H
#define APRIL_WINP8_APP_LAUNCHER_H

#include <hltypes/hplatform.h>

using namespace Windows::ApplicationModel::Core;

namespace april
{
	ref class WinP8_AppLauncher : public IFrameworkViewSource
	{
	public:
		virtual IFrameworkView^ CreateView();

	};

}

#endif
#endif