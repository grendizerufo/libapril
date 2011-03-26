/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <IL/il.h>

#include <hltypes/util.h>

#include "ImageSource.h"
#include "RenderSystem.h"

namespace april
{
	ImageSource::ImageSource()
	{
		ilGenImages(1, &mImageId);
		this->compressedLength = 0;
	}
	
	ImageSource::~ImageSource()
	{
		ilDeleteImages(1, &mImageId);
	}
	
	Color ImageSource::getPixel(int x, int y)
	{
		x = hclamp(x, 0, w - 1);
		y = hclamp(y, 0, h - 1);
		Color c = Color::WHITE;
		int index = x + y * w;
		c.r = this->data[index * this->bpp];
		c.g = this->data[index * this->bpp + 1];
		c.b = this->data[index * this->bpp + 2];
		if (this->bpp == 4) // RGBA
		{
			c.a = this->data[index * this->bpp + 3];
		}
		return c;
	}
	
	void ImageSource::setPixel(int x, int y, Color c)
	{
		x = hclamp(x, 0, w - 1);
		y = hclamp(y, 0, h - 1);
		int index = x + y * w;
		this->data[index * this->bpp] = c.r;
		this->data[index * this->bpp + 1] = c.g;
		this->data[index * this->bpp + 2] = c.b;
		if (this->bpp == 4) // RGBA
		{
			this->data[index * 4 + 3] = c.a;
		}
	}

	Color ImageSource::getInterpolatedPixel(float x, float y)
	{
		return getPixel((int)x, (int)y); // TODO
	}
	
	void ImageSource::copyPixels(void* output, int format)
	{
		ilBindImage(this->getImageId());
		ilCopyPixels(0, 0, 0, w, h, 1, format, IL_UNSIGNED_BYTE, output);
	}
	
	void ImageSource::setPixels(int x, int y, int w, int h, Color c)
	{
		ilBindImage(this->getImageId());
		w = hmax(w, 1);
		h = hmax(h, 1);
		int size = w * h;
		int baseSize = this->bpp * sizeof(unsigned char);
		unsigned char color[4] = {c.r, c.g, c.b, c.a};
		unsigned char* data = new unsigned char[size * this->bpp];
		// using memory duplication instead of linear copying
		memcpy(data, color, baseSize);
		for (int i = 1; i < size; i *= 2)
		{
			memcpy(&data[i * this->bpp], data, hmin(i, size - i) * baseSize);
		}
		ilSetPixels(x, y, 0, w, h, 1, IL_RGBA, IL_UNSIGNED_BYTE, data);
		delete [] data;
	}

	void ImageSource::copyImage(ImageSource* other)
	{
		memcpy(this->data, other->data, this->w * this->h * this->bpp * sizeof(unsigned char));
	}

	void ImageSource::blit(int x, int y, ImageSource* other, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->w - 1);
		y = hclamp(y, 0, this->h - 1);
		sx = hclamp(sx, 0, other->w - 1);
		sy = hclamp(sy, 0, other->h - 1);
		sw = hmin(sw, hmin(this->w - x, other->w - sx));
		sh = hmin(sh, hmin(this->h - y, other->h - sy));
		unsigned char* c;
		unsigned char* sc;
		unsigned char a;
		for (int j = 0; j < sh; j++)
		{
			for (int i = 0; i < sw; i++)
			{
				c = &this->data[((x + i) + (y + j) * this->w) * this->bpp];
				sc = &other->data[((sx + i) + (sy + j) * other->w) * other->bpp];
				a = sc[3] * alpha / 255;
				c[0] = (sc[0] * a + (255 - a) * c[0]) / 255;
				c[1] = (sc[1] * a + (255 - a) * c[1]) / 255;
				c[2] = (sc[2] * a + (255 - a) * c[2]) / 255;
				c[3] = hmax(c[3], a);
			}
		}
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
		img->format = ilGetInteger(IL_IMAGE_FORMAT);
		img->data = ilGetData();
		return img;
	}

	ImageSource* createEmptyImage(int w, int h)
	{
		ImageSource* img = new ImageSource();
		ilBindImage(img->getImageId());
		int size = w * h * 4;
		unsigned char* data = new unsigned char[size];
		memset(data, 0, size * sizeof(unsigned char));
		ilTexImage(w, h, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, data);
		delete [] data;
		img->w = ilGetInteger(IL_IMAGE_WIDTH);
		img->h = ilGetInteger(IL_IMAGE_HEIGHT);
		img->bpp = ilGetInteger(IL_IMAGE_BPP);
		img->format = ilGetInteger(IL_IMAGE_FORMAT);
		img->data = ilGetData();
		return img;
	}

}
