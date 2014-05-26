/// @file
/// @version 3.36
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WIN32_WINDOW
#include <hltypes/hstring.h>

#include "Win32_Cursor.h"

namespace april
{
	Win32_Cursor::Win32_Cursor() : Cursor(), cursor(NULL)
	{
	}

	Win32_Cursor::~Win32_Cursor()
	{
		if (this->cursor != NULL)
		{
			DestroyCursor(this->cursor);
			this->cursor = NULL;
		}
	}

	bool Win32_Cursor::_create(chstr filename)
	{
		if (!Cursor::_create(filename))
		{
			return false;
		}
		if (filename == "")
		{
			return false;
		}
		this->cursor = LoadCursorFromFileW(filename.w_str().c_str());
		return true;
	}

}
#endif
