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

#ifdef _ANDROID
#ifndef APRIL_ANDROIDJNI_KEYS_H
#define APRIL_ANDROIDJNI_KEYS_H

#include "Keys.h"

namespace april
{
	Key android2april(int androidKeyCode);
	void initAndroidKeyMap();
}

#endif
#endif
