/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_RENDERSYSTEM_H
#define APRIL_RENDERSYSTEM_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>
#include <gtypes/Matrix4.h>

#include "aprilExport.h"

enum TextureType
{
	AT_NORMAL = 1,
	AT_RENDER_TARGET = 2
};

enum TextureFormat
{
	AT_XRGB = 1,
	AT_ARGB = 2,
    AT_RGB = 3
};

namespace april
{
	class Window;
	class ImageSource;
	// render operations
	enum RenderOp
	{
		TriangleList = 1,
		TriangleStrip = 2,
		TriangleFan = 3,
		LineList = 4,
		LineStrip = 5,
		PointList = 6,
	};
	
	enum TextureFilter
	{
		Nearest = 1,
		Linear = 2
	};
	
	enum BlendMode
	{
		ALPHA_BLEND,
		ADD,
		DEFAULT
	};
	
	struct aprilExport PlainVertex : public gvec3
	{
	public:
		void operator=(const gvec3& v);
	};

	struct aprilExport ColoredVertex : public PlainVertex
	{
	public:
		unsigned int color;
	};

	class aprilExport TexturedVertex : public PlainVertex
	{
	public:
		float u;
		float v;
	};

	class aprilExport ColoredTexturedVertex : public ColoredVertex
	{
	public:
		float u;
		float v;
	};
    
    class aprilExport ColoredTexturedNormalVertex : public ColoredTexturedVertex
    {
    public:
        gvec3 normal;
    };
    
    class aprilExport TexturedNormalVertex : public TexturedVertex
    {
    public:
        gvec3 normal;
    };
    
    class aprilExport ColoredNormalVertex : public ColoredVertex
    {
    public:
        gvec3 normal;
    };

	class aprilExport Color
	{
	public:
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
		
		Color(int r, int g, int b, int a = 255);
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		Color(unsigned int color);
		Color(chstr hex);
		Color();

		void set(int r, int g, int b, int a = 255);
		void set(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
		void set(unsigned int color);
		void set(chstr hex);

		float r_f() { return r / 255.0f; }
		float g_f() { return g / 255.0f; }
		float b_f() { return b / 255.0f; }
		float a_f() { return a / 255.0f; }
		
		hstr hex(bool rgbOnly = false); // careful when using rgbOnly!
		
		operator unsigned int() const;
		
		bool operator==(Color& other);
		bool operator!=(Color& other);
		
		Color operator+(Color& other);
		Color operator-(Color& other);
		Color operator*(Color& other);
		Color operator/(Color& other);
		Color operator*(float value);
		Color operator/(float value);
		Color operator+=(Color& other);
		Color operator-=(Color& other);
		Color operator*=(Color& other);
		Color operator/=(Color& other);
		Color operator*=(float value);
		Color operator/=(float value);
		
		// DEPRECATED
		Color(float r, float g, float b, float a = 1.0f) DEPRECATED_ATTRIBUTE;
		void setColor(float r, float g, float b, float a = 1.0f) DEPRECATED_ATTRIBUTE;
		void setColor(int r, int g, int b, int a = 255) DEPRECATED_ATTRIBUTE;
		void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) DEPRECATED_ATTRIBUTE;
		void setColor(unsigned int color) DEPRECATED_ATTRIBUTE;
		void setColor(chstr hex) DEPRECATED_ATTRIBUTE;
		void set(float r, float g, float b, float a = 1.0f) DEPRECATED_ATTRIBUTE;
		
		// predefined colors
		#define RED			Color(255,   0,   0)
		#define GREEN		Color(  0, 255,   0)
		#define BLUE		Color(  0,   0, 255)
		#define YELLOW		Color(255, 255,   0)
		#define MANGENTA	Color(255,   0, 255)
		#define CYAN		Color(  0, 255, 255)
		#define ORANGE		Color(255, 127,   0)
		#define PINK		Color(255,   0, 127)
		#define TEAL		Color(  0, 255, 127)
		#define NEON		Color(127, 255,   0)
		#define PURPLE		Color(127,   0, 255)
		#define AQUA		Color(  0, 127, 255)
		#define WHITE		Color(255, 255, 255)
		#define GREY		Color(127, 127, 127)
		#define BLACK		Color(  0,   0,   0)
		#define CLEAR		Color(  0,   0,   0,   0)
		
	};

	class aprilExport Texture
	{
	public:
		Texture();
		virtual ~Texture();
		virtual void unload() = 0;
		virtual int getSizeInBytes() = 0;
		
		virtual Color getPixel(int x, int y);
		virtual Color getInterpolatedPixel(float x, float y);
		
		void addDynamicLink(Texture* lnk);
		void removeDynamicLink(Texture* lnk);
		void _resetUnusedTimer(bool recursive = true);
		
		int getWidth() { return mWidth; };
		int getHeight() { return mHeight; };
		/// only used with dynamic textures since at chapter load you need it's dimensions for images, but you don't know them yet
		void _setDimensions(int w, int h) { mWidth = w; mHeight = h; }
		bool isDynamic() { return mDynamic; }
		virtual bool isLoaded() = 0;
		
		void update(float time_increase);
		hstr getFilename() { return mFilename; }
		
		void setTextureFilter(TextureFilter filter) { mTextureFilter = filter; }
		void setTextureWrapping(bool wrap) { mTextureWrapping = wrap; }
		bool isTextureWrappingEnabled() { return mTextureWrapping; }
		TextureFilter getTextureFilter() { return mTextureFilter; }
		
	protected:
		bool mDynamic;
		hstr mFilename;
		int mWidth;
		int mHeight;
		float mUnusedTimer;
		TextureFilter mTextureFilter;
		bool mTextureWrapping;
		harray<Texture*> mDynamicLinks;
		
	};
	
	class aprilExport RAMTexture : public Texture
	{
	public:
		RAMTexture(chstr filename, bool dynamic);
		virtual ~RAMTexture();
		void load();
		void unload();
		bool isLoaded();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color c);
		Color getInterpolatedPixel(float x, float y);
		int getSizeInBytes();
		
	protected:
		ImageSource* mBuffer;
		
	};

	class aprilExport RenderSystem
	{
	public:
		RenderSystem();
		virtual ~RenderSystem();

		// object creation
		hstr findTextureFile(chstr filename);
		virtual Texture* loadTexture(chstr filename, bool dynamic = false) = 0;
		Texture* loadRAMTexture(chstr filename, bool dynamic = false);
		virtual Texture* createTextureFromMemory(unsigned char* rgba, int w, int h) = 0;
		virtual Texture* createEmptyTexture(int w, int h, TextureFormat fmt = AT_XRGB, TextureType type = AT_NORMAL) = 0;

		// modelview matrix transformation
		void setIdentityTransform();
		void translate(float x, float y, float z = 0.0f);
		void rotate(float angle, float ax = 0.0f, float ay = 0.0f, float az = -1.0f);
		void scale(float s);
		void scale(float sx, float sy, float sz);
		// camera functions
		void lookAt(const gvec3 &eye, const gvec3 &direction, const gvec3 &up);
		// projection matrix tronsformation
		void setOrthoProjection(float w, float h, float x_offset = 0.0f, float y_offset = 0.0f);
		void setPerspective(float fov, float aspect, float nearClip, float farClip);
		// rendersys matrix operations
		void setModelviewMatrix(const gmat4& matrix);
		void setProjectionMatrix(const gmat4& matrix);
		virtual bool isFullscreen() { return false; } //2DO - implement in derived classes
		virtual void setFullscreen(bool fullscreen) { } //2DO - implement in derived classes
		
		const gmat4& getModelviewMatrix();
		const gmat4& getProjectionMatrix();
		// render state
		virtual void setBlendMode(BlendMode mode) = 0;
		// caps
		virtual float getPixelOffset() = 0;
		virtual hstr getName() = 0;
		// rendering
		virtual void clear(bool color = true, bool depth = false) = 0;
		virtual void setTexture(Texture* t) = 0;
		virtual void render(RenderOp renderOp, TexturedVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, TexturedVertex* v, int nVertices,Color color) = 0;
		virtual void render(RenderOp renderOp, PlainVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, PlainVertex* v, int nVertices,Color color) = 0;
		virtual void render(RenderOp renderOp, ColoredVertex* v, int nVertices) = 0;
		
		virtual void setRenderTarget(Texture* source) = 0;
		
		void drawColoredQuad(grect rect, Color color);
		void drawTexturedQuad(grect rect, grect src);
		void drawTexturedQuad(grect rect, grect src, Color color);

		
		float getIdleTextureUnloadTime() { return mIdleUnloadTime; }
		void setIdleTextureUnloadTime(float time) { mIdleUnloadTime = time; }

		virtual void setAlphaMultiplier(float value) = 0;
		float getAlphaMultiplier() { return mAlphaMultiplier; }


		virtual void beginFrame() = 0;

		void forceDynamicLoading(bool value) { mDynamicLoading = value; }
		bool isDynamicLoadingForced() { return mDynamicLoading; }
		
		Window* getWindow() { return mWindow; }
		
        virtual ImageSource* grabScreenshot() = 0;
        
		virtual void presentFrame();

	protected:
		Window* mWindow;
		float mAlphaMultiplier;
		float mIdleUnloadTime;
		bool mDynamicLoading;
		TextureFilter mTextureFilter;
		bool mTextureWrapping;
		gmat4 mModelviewMatrix;
		gmat4 mProjectionMatrix;
		
		virtual void _setModelviewMatrix(const gmat4& matrix) = 0;
		virtual void _setProjectionMatrix(const gmat4& matrix) = 0;
		
	};

	aprilFnExport void setLogFunction(void (*fnptr)(chstr));
	aprilFnExport void init(chstr rendersystem_name, int w, int h, bool fullscreen, chstr title);
	aprilFnExport void destroy();
	aprilFnExport void addTextureExtension(chstr extension);
	aprilFnExport void log(chstr message,chstr prefix = "[april] ");
	aprilFnExport void logf(chstr message, ...);
	
	// global rendersys shortcut variable
	aprilFnExport extern april::RenderSystem* rendersys;
	
}

#endif