/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>

#include "aprilUtil.h"

#define HSTR_SEPARATOR ','

namespace april
{
	RenderOperation TriangleList = RO_TRIANGLE_LIST; // DEPRECATED
	RenderOperation TriangleStrip = RO_TRIANGLE_STRIP; // DEPRECATED
	RenderOperation TriangleFan = RO_TRIANGLE_FAN; // DEPRECATED
	RenderOperation LineList = RO_LINE_LIST; // DEPRECATED
	RenderOperation LineStrip = RO_LINE_STRIP; // DEPRECATED
	RenderOperation PointList = RO_POINT_LIST; // DEPRECATED
	RenderOperation RENDER_OP_UNDEFINED = RO_UNDEFINED; // DEPRECATED
	BlendMode DEFAULT = BM_DEFAULT; // DEPRECATED
	BlendMode ALPHA_BLEND = BM_ALPHA; // DEPRECATED
	BlendMode ADD = BM_ADD; // DEPRECATED
	BlendMode SUBTRACT = BM_SUBTRACT; // DEPRECATED
	BlendMode OVERWRITE = BM_OVERWRITE; // DEPRECATED
	BlendMode BLEND_MODE_UNDEFINED = BM_UNDEFINED; // DEPRECATED
	ColorMode NORMAL = CM_DEFAULT; // DEPRECATED
	ColorMode MULTIPLY = CM_MULTIPLY; // DEPRECATED
	ColorMode LERP = CM_LERP; // DEPRECATED
	ColorMode ALPHA_MAP = CM_ALPHA_MAP; // DEPRECATED
	ColorMode COLOR_MODE_UNDEFINED = CM_UNDEFINED; // DEPRECATED

	void PlainVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void TexturedVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredTexturedVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredTexturedNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void TexturedNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void ColoredNormalVertex::operator=(const gvec3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void rgbToHsl(unsigned char r, unsigned char g, unsigned char b, float* h, float* s, float* l)
	{
		int min = hmin(hmin(r, g), b);
		int max = hmax(hmax(r, g), b);
		int delta = max - min;
		*h = *s = 0.0f;
		*l = (max + min) / 510.0f;
		if (delta > 0)
		{
			if (*l > 0.0f && *l < 1.0f)
			{
				*s = (delta / 255.0f) / (*l < 0.5f ? (2 * *l) : (2 - 2 * *l));
			}
			if (max == r)
			{
				*h = (g - b) / (float)delta;
				if (g < b)
				{
					*h += 6.0f;
				}
			}
			else if (max == g)
			{
				*h += (b - r) / (float)delta + 2.0f;
			}
			else if (max == b)
			{
				*h += (r - g) / (float)delta + 4.0f;
			}
			*h *= 0.16666667f;
		}
	}

	float _colorHueToRgb(float p, float q, float h)
	{ 
		h = (h < 0 ? h + 1 : ((h > 1) ? h - 1 : h));
		if (h * 6 < 1)
		{
			return p + (q - p) * h * 6;
		}
		if (h * 2 < 1)
		{
			return q;
		}
		if (h * 3 < 2)
		{
			return (p + (q - p) * (0.6666667f - h) * 6);
		}
		return p;
	}

	void hslToRgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b)
	{
		if (s == 0.0f)
		{
			*r = *g = *b = (unsigned char)(l * 255);
			return;
		}
		float q = (l < 0.5f ? l * (1 + s) : l + s - l * s);
		float p = l * 2 - q;
		*r = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h + 0.3333333f));
		*g = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h));
		*b = (unsigned char)hround(255.0f * _colorHueToRgb(p, q, h - 0.3333333f));
	}
	
	hstr generateName(chstr prefix)
	{
		static hmap<hstr, int> counters;
		int count = counters[prefix] + 1;
		counters[prefix] = count;
		return prefix.replaced(".", "_") + hstr(count);
	}

	hstr gvec2ToHstr(gvec2 vector)
	{
		return hsprintf("%f%c%f", vector.x, HSTR_SEPARATOR, vector.y);
	}

	hstr gvec3ToHstr(gvec3 vector)
	{
		return hsprintf("%f%c%f%c%f", vector.x, HSTR_SEPARATOR, vector.y, HSTR_SEPARATOR, vector.z);
	}

	hstr grectToHstr(grect rect)
	{
		return hsprintf("%f%c%f%c%f%c%f", rect.x, HSTR_SEPARATOR, rect.y, HSTR_SEPARATOR, rect.w, HSTR_SEPARATOR, rect.h);
	}

	gvec2 hstrToGvec2(chstr string)
	{
		harray<hstr> data = string.split(HSTR_SEPARATOR);
		if (data.size() != 2)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Vector2.");
		}
		return gvec2(data[0].trimmed(), data[1].trimmed());
	}

	gvec3 hstrToGvec3(chstr string)
	{
		harray<hstr> data = string.split(HSTR_SEPARATOR);
		if (data.size() != 3)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Vector3.");
		}
		return gvec3(data[0].trimmed(), data[1].trimmed(), data[2].trimmed());
	}

	grect hstrToGrect(chstr string)
	{
		harray<hstr> data = string.split(HSTR_SEPARATOR);
		if (data.size() != 4)
		{
			throw Exception("Cannot convert string '" + string + "' to gtypes::Rectangle.");
		}
		return grect(data[0].trimmed(), data[1].trimmed(), data[2].trimmed(), data[3].trimmed());
	}

}
