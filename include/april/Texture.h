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
/// Defines a generic texture.

#ifndef APRIL_TEXTURE_H
#define APRIL_TEXTURE_H

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <hltypes/hstream.h>

#include "aprilExport.h"
#include "Color.h"
#include "Image.h"

namespace april
{
	class Image;
	class RenderSystem;
	class TextureAsync;
	
	class aprilExport Texture
	{
	public:
		friend class RenderSystem;
		friend class TextureAsync;

		enum Type
		{
			/// @brief Resides in RAM and on GPU, can be modified. Best used for manually created textures or loaded from files which will be modified.
			TYPE_MANAGED = 1,
			/// @brief Cannot be modified or read. Texture with manual data will have a copy of the data in RAM, files will be reloaded from persistent memory.
			TYPE_IMMUTABLE = 2,
			/// @brief Used for feeding the GPU texture data constantly (e.g. video). It has no local RAM copy for when the rendering context is lost and cannot be restored.
			TYPE_VOLATILE = 3,
			/// @brief Used for render targets. Acts like MANAGED.
			// TODOaa - may not be implemented on all platforms yet
			TYPE_RENDER_TARGET = 4
		};

		enum Filter
		{
			FILTER_NEAREST = 1,
			FILTER_LINEAR = 2,
			FILTER_UNDEFINED = 0x7FFFFFFF
		};

		enum AddressMode
		{
			ADDRESS_WRAP = 0,
			ADDRESS_CLAMP = 1,
			ADDRESS_UNDEFINED = 0x7FFFFFFF
		};

		enum LoadMode
		{
			/// @brief Loads the texture and uploads data to the GPU right away.
			LOAD_IMMEDIATE = 0,
			/// @brief Doesn't load the texture yet at all. It will be loaded and uploaded to the GPU on the first use.
			LOAD_ON_DEMAND = 1,
			/// @brief Loads the texture asynchronously right away and uploads the data to the GPU as soon as it is available before the next frame.
			/// @note If the texture is used before it loaded asynchronously, it cannot be uploaded to the GPU and will not be rendered properly.
			LOAD_ASYNC = 2,
			/// @brief Loads the texture asynchronously right away, but it will upload the data to the GPU on the first use.
			/// @note If the texture is used before it loaded asynchronously, it cannot be uploaded to the GPU and will not be rendered properly.
			LOAD_ASYNC_ON_DEMAND = 3
		};

		DEPRECATED_ATTRIBUTE static Image::Format FORMAT_ALPHA;
		DEPRECATED_ATTRIBUTE static Image::Format FORMAT_ARGB;

		virtual ~Texture();

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(LoadMode, loadMode, LoadMode);
		HL_DEFINE_GET(Image::Format, format, Format);
		HL_DEFINE_GETSET(Filter, filter, Filter);
		HL_DEFINE_GETSET(AddressMode, addressMode, AddressMode);
		HL_DEFINE_IS(locked, Locked);
		HL_DEFINE_IS(dirty, Dirty);
		HL_DEFINE_IS(fromResource, FromResource);
		int getWidth();
		int getHeight();
		int getBpp();
		int getByteSize();
		int getCurrentVRamSize();
		int getCurrentRamSize();
		int getCurrentAsyncRamSize();
		bool isLoaded();
		bool isLoadedAsync();
		bool isAsyncLoadQueued();

		bool load();
		bool loadAsync();
		void unload();
		bool loadMetaData();
		/// @note A timeout value of 0.0 means indefinitely.
		void waitForAsyncLoad(float timeout = 0.0f);

		bool lock();
		bool unlock();

		bool clear();
		Color getPixel(int x, int y);
		Color getPixel(gvec2 position);
		bool setPixel(int x, int y, Color color);
		bool setPixel(gvec2 position, Color color);
		Color getInterpolatedPixel(float x, float y);
		Color getInterpolatedPixel(gvec2 position);
		bool fillRect(int x, int y, int w, int h, Color color);
		bool fillRect(grect rect, Color color);
		bool copyPixelData(unsigned char** output, Image::Format format);
		bool copyPixelData(unsigned char** output);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture);
		bool write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool write(grect srcRect, gvec2 destPosition, Texture* texture);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Image* image);
		bool write(grect srcRect, gvec2 destPosition, Image* image);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture);
		bool writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(grect srcRect, grect destRect, Texture* texture);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image);
		bool writeStretch(grect srcRect, grect destRect, Image* image);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, Texture* texture, unsigned char alpha = 255);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* image, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, Image* image, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, Texture* texture, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* image, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, Image* image, unsigned char alpha = 255);
		bool rotateHue(int x, int y, int w, int h, float degrees);
		bool rotateHue(grect rect, float degrees);
		bool saturate(int x, int y, int w, int h, float factor);
		bool saturate(grect rect, float factor);
		bool invert(int x, int y, int w, int h);
		bool invert(grect rect);
		/// @note srcData must be the same width and height as the image
		bool insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity);
		bool insertAlphaMap(Texture* texture, unsigned char median, int ambiguity);
		bool insertAlphaMap(Image* image, unsigned char median, int ambiguity);

	protected:
		struct aprilExport Lock
		{
		public:
			void* systemBuffer;
			int x;
			int y;
			int w;
			int h;
			int dx;
			int dy;
			unsigned char* data;
			int dataWidth;
			int dataHeight;
			Image::Format format;
			bool locked;
			bool failed;
			bool renderTarget;

			Lock();
			~Lock();

			void activateFail();
			void activateLock(int x, int y, int w, int h, int dx, int dy, unsigned char* data, int dataWidth, int dataHeight, Image::Format format);
			void activateRenderTarget(int x, int y, int w, int h, int dx, int dy, unsigned char* data, int dataWidth, int dataHeight, Image::Format format);

		};

		Texture(bool fromResource);

		hstr filename;
		Type type;
		bool loaded;
		LoadMode loadMode;
		Image::Format format;
		unsigned int dataFormat; // used internally for special image data formatting
		int width;
		int height;
		float effectiveWidth; // used only with software NPOT textures
		float effectiveHeight; // used only with software NPOT textures
		int compressedSize; // used in compressed textures only
		Filter filter;
		AddressMode addressMode;
		bool locked;
		bool dirty;
		unsigned char* data;
		unsigned char* dataAsync;
		bool asyncLoadQueued;
		bool asyncLoadDiscarded;
		hmutex asyncLoadMutex;
		bool fromResource;
		bool firstUpload; // required because of how some rendering systems work

		virtual bool _create(chstr filename, Type type, LoadMode loadMode);
		virtual bool _create(chstr filename, Image::Format format, Type type, LoadMode loadMode);
		virtual bool _create(int w, int h, unsigned char* data, Image::Format format, Type type);
		virtual bool _create(int w, int h, Color color, Image::Format format, Type type);

		virtual bool _createInternalTexture(unsigned char* data, int size, Type type) = 0;
		virtual bool _destroyInternalTexture() = 0;
		virtual void _assignFormat() = 0;

		hstream* _prepareAsyncStream();
		void _decodeFromAsyncStream(hstream* stream);

		hstr _getInternalName();

		void _setupPot(int& outWidth, int& outHeight);
		unsigned char* _createPotData(int& outWidth, int& outHeight, unsigned char* data);
		unsigned char* _createPotClearData(int& outWidth, int& outHeight);

		Lock _tryLock(int x, int y, int w, int h);
		Lock _tryLock();
		bool _unlock(Lock lock, bool update);
		virtual Lock _tryLockSystem(int x, int y, int w, int h) = 0;
		virtual bool _unlockSystem(Lock& lock, bool update) = 0;
		bool _uploadDataToGpu(int x, int y, int w, int h);
		virtual bool _uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat) = 0;

	};
	
}

#endif
