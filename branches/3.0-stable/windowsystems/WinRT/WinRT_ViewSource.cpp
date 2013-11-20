/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW

#include <hltypes/hplatform.h>

#include "WinRT_View.h"
#include "WinRT_ViewSource.h"

namespace april
{
	IFrameworkView^ WinRT_ViewSource::CreateView()
	{
		return ref new WinRT_View();
	}
	
}
#endif