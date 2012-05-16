/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef USE_IL
#include <IL/il.h>

#include <hltypes/hltypesUtil.h>

#include "ImageSource.h"
#include "RenderSystem.h"

namespace april
{
	ImageSource::ImageSource()
	{
		ilGenImages(1, &this->imageId);
		this->compressedLength = 0;
	}
	
	ImageSource::~ImageSource()
	{
		ilDeleteImages(1, &this->imageId);
	}

	void ImageSource::copyImage(ImageSource* source, int bpp)
	{
		ilBindImage(source->getImageId());
		ilCopyPixels(0, 0, 0, this->w, this->h, 1, this->format, IL_UNSIGNED_BYTE, this->data);
	}

	ImageSource* loadImage(chstr filename)
	{
		ImageSource* img = new ImageSource();
		ilBindImage(img->getImageId());
		int success = ilLoadImage(filename.c_str());
		if (!success)
		{
			delete img;
			return NULL;
		}
		img->w = ilGetInteger(IL_IMAGE_WIDTH);
		img->h = ilGetInteger(IL_IMAGE_HEIGHT);
		img->bpp = ilGetInteger(IL_IMAGE_BPP);
		img->format = (ilGetInteger(IL_IMAGE_FORMAT) == 6408 ? AF_RGBA : AF_RGB);
		img->data = ilGetData();
		return img;
	}

}
#endif