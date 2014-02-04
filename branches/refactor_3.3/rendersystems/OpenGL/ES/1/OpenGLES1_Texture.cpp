/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGLES1
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "OpenGLES1_Texture.h"

namespace april
{
	OpenGLES1_Texture::OpenGLES1_Texture(chstr filename) : OpenGLES_Texture(filename)
	{
	}

	OpenGLES1_Texture::OpenGLES1_Texture(int w, int h, unsigned char* rgba) : OpenGLES_Texture(w, h, rgba)
	{
	}

	OpenGLES1_Texture::OpenGLES1_Texture(int w, int h, Format format, Type type, Color color) :
		OpenGLES_Texture(w, h, format, type, color)
	{
	}

	OpenGLES1_Texture::~OpenGLES1_Texture()
	{
	}

}

#endif