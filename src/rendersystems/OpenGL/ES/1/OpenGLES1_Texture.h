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
/// Defines an OpenGLES1 specific texture.

#ifdef _OPENGLES1
#ifndef APRIL_OPENGLES1_TEXTURE_H
#define APRIL_OPENGLES1_TEXTURE_H

#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGLES_Texture.h"

namespace april
{
	class OpenGLES1_Texture : public OpenGLES_Texture
	{
	public:
		OpenGLES1_Texture(bool fromResource);
		~OpenGLES1_Texture();

	};

}

#endif
#endif
