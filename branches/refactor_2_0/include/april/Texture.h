/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic texture.

#ifndef APRIL_TEXTURE_H
#define APRIL_TEXTURE_H

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Color.h"
#include "aprilExport.h"

namespace april
{
	class ImageSource;
	
	class aprilExport Texture
	{
	public:
		friend class RenderSystem;

		enum Type
		{
			TYPE_NORMAL = 1,
			TYPE_RENDER_TARGET = 2
		};

		enum Format
		{
			FORMAT_INVALID = 0,
			FORMAT_RGB = 1,
			FORMAT_RGBA = 2/*,
			FORMAT_PALETTE = 3,
			FORMAT_MONOCHROME = 4*/
		};

		enum Filter
		{
			FILTER_NEAREST = 1,
			FILTER_LINEAR = 2
		};

		enum AddressMode
		{
			ADDRESS_WRAP = 0,
			ADDRESS_CLAMP = 1
		};
	
		Texture();
		virtual ~Texture();
		virtual bool load() = 0;
		virtual void unload() = 0;

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		HL_DEFINE_GET(int, bpp, Bpp);
		HL_DEFINE_GETSET(Filter, filter, Filter);
		HL_DEFINE_GETSET(AddressMode, addressMode, AddressMode);
		HL_DEFINE_IS(bool, dynamic, Dynamic);
		HL_DEFINE_GET(float, unusedTime, UnusedTime);
		int getByteSize();

		virtual bool isLoaded() = 0;
		
		void addDynamicLink(Texture* link);
		void removeDynamicLink(Texture* link);
		
		virtual void clear();
		virtual Color getPixel(int x, int y);
		virtual void setPixel(int x, int y, Color color);
		virtual void fillRect(int x, int y, int w, int h, Color color);
		virtual void blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		virtual void blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		virtual void stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		virtual void stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		virtual void rotateHue(float degrees);
		virtual void saturate(float factor);
		virtual void insertAsAlphaMap(Texture* source, unsigned char median, int ambiguity);

		Color getPixel(gvec2 position);
		void setPixel(gvec2 position, Color color);
		Color getInterpolatedPixel(float x, float y);
		Color Texture::getInterpolatedPixel(gvec2 position);
		void fillRect(grect rect, Color color);
		void blit(int x, int y, ImageSource* image, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void blit(gvec2 position, Texture* texture, grect source, unsigned char alpha = 255);
		void blit(gvec2 position, ImageSource* image, grect source, unsigned char alpha = 255);
		void blit(gvec2 position, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, grect source, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, ImageSource* image, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(grect destination, Texture* texture, grect source, unsigned char alpha = 255);
		void stretchBlit(grect destination, ImageSource* image, grect source, unsigned char alpha = 255);
		void stretchBlit(grect destination, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, grect source, unsigned char alpha = 255);

		void update(float k);
		
		DEPRECATED_ATTRIBUTE int getSizeInBytes() { return this->getByteSize(); }
		DEPRECATED_ATTRIBUTE void setTextureFilter(Filter value) { this->setFilter(value); }
		DEPRECATED_ATTRIBUTE void setTextureWrapping(bool wrap) { this->setAddressMode(wrap ? ADDRESS_WRAP : ADDRESS_CLAMP); }
		DEPRECATED_ATTRIBUTE bool isTextureWrappingEnabled() { return (this->getAddressMode() == ADDRESS_WRAP); }
		DEPRECATED_ATTRIBUTE Filter getTextureFilter() { return this->getFilter(); }
		DEPRECATED_ATTRIBUTE bool isValid() { return this->isLoaded(); }
		
	protected:
		hstr filename;
		int width;
		int height;
		int bpp;
		Filter filter;
		AddressMode addressMode;
		bool dynamic;
		float unusedTime;
		harray<Texture*> dynamicLinks;

		hstr _getInternalName();

		hstr _findTextureFilename(chstr filename);
		void _resetUnusedTime();
		
		// warning: these may currently not work well with anything else than DirectX9
		void _blit(unsigned char* thisData, int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void _stretchBlit(unsigned char* thisData, int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);

	};
	
}

#endif
