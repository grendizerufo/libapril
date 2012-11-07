/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>
#include <gtypes/Matrix4.h>
#include "aprilExport.h"


namespace gtypes
{
	class Vector2;
}

enum TextureType
{
	AT_NORMAL=1,
	AT_RENDER_TARGET=2
};

enum TextureFormat
{
	AT_XRGB=1,
	AT_ARGB=2,
    AT_RGB=3
};

namespace april
{
	class Window;
	class ImageSource;
	// render operations
	enum RenderOp
	{
		TriangleList=1,
		TriangleStrip=2,
		TriangleFan=3,
		LineList=4,
		LineStrip=5,
		PointList=6,
	};
	
	enum TextureFilter
	{
		Nearest=1,
		Linear=2
	};
	
	enum BlendMode
	{
		ALPHA_BLEND,
		ADD,
		DEFAULT
	};

	struct aprilExport DisplayMode
	{
		int width;
		int height;
		int refreshRate;

		bool operator==(const DisplayMode& other)
		{
			return (this->width == other.width && this->height == other.height && this->refreshRate == other.refreshRate);
		}

		bool operator!=(const DisplayMode& other)
		{
			return !(*this == other);
		}
	};
	
	struct aprilExport PlainVertex : public gtypes::Vector3
	{
	public:
		void operator=(const gtypes::Vector3& v);
	};

	struct aprilExport ColoredVertex : public PlainVertex
	{
	public:
		unsigned int color;
	};

	class aprilExport TexturedVertex : public PlainVertex
	{
	public:
		float u,v;
	};

	class aprilExport ColoredTexturedVertex : public PlainVertex
	{
	public:
		unsigned int color;
		float u,v;
	};
    
    class aprilExport ColoredTexturedNormalVertex : public PlainVertex
    {
    public:
        float u,v;
        unsigned int color;
        gtypes::Vector3 normal;
        
    };
    
    class aprilExport TexturedNormalVertex : public PlainVertex
    {
    public:
        float u,v;
        gtypes::Vector3 normal;
        
    };
    
    class aprilExport ColoredNormalVertex : public PlainVertex
    {
    public:
        unsigned int color;
        gtypes::Vector3 normal;
        
    };

	class aprilExport Color
	{
	public:
		unsigned char r,g,b,a;
		Color(float a,float r,float g,float b);
		Color(int a,int r,int g,int b);
		Color(unsigned char a,unsigned char r,unsigned char g,unsigned char b);
		Color(unsigned int color);
		Color(chstr hex);
		Color();

		void setColor(float a,float r,float g,float b);
		void setColor(int a,int r,int g,int b);
		void setColor(unsigned char a,unsigned char r,unsigned char g,unsigned char b);
		void setColor(unsigned int color);
		void setColor(chstr hex);

		float r_float() { return r/255.0f; }
		float g_float() { return g/255.0f; }
		float b_float() { return b/255.0f; }
		float a_float() { return a/255.0f; }
		
		bool operator==(Color& other);
		bool operator!=(Color& other);
		
		Color operator+(Color& other);
		Color operator-(Color& other);
		Color operator*(Color& other);
		Color operator/(Color& other);
		Color operator*(float value);
		Color operator/(float value);
		void operator+=(Color& other);
		void operator-=(Color& other);
		void operator*=(Color& other);
		void operator/=(Color& other);
		void operator*=(float value);
		void operator/=(float value);
		
	};

	class aprilExport Texture
	{
	protected:
		bool mDynamic;
		hstr mFilename;
		int mWidth,mHeight;
		float mUnusedTimer;
		TextureFilter mTextureFilter;
		bool mTextureWrapping;
		harray<Texture*> mDynamicLinks;
	public:
		Texture();
		virtual ~Texture();
		virtual void unload()=0;
		virtual int getSizeInBytes()=0;
		
		virtual Color getPixel(int x,int y);
		virtual Color getInterpolatedPixel(float x,float y);
		
		void addDynamicLink(Texture* lnk);
		void removeDynamicLink(Texture* lnk);
		void _resetUnusedTimer(bool recursive=1);
		
		int getWidth() { return mWidth; };
		int getHeight() { return mHeight; };
		/// only used with dynamic textures since at chapter load you need it's dimensions for images, but you don't know them yet
		void _setDimensions(int w,int h) { mWidth=w; mHeight=h; }
		bool isDynamic() { return mDynamic; }
		virtual bool isLoaded()=0;
		
		void update(float time_increase);
		hstr getFilename() { return mFilename; }
		
		void setTextureFilter(TextureFilter filter) { mTextureFilter=filter; }
		void setTextureWrapping(bool wrap) { mTextureWrapping=wrap; }
		bool isTextureWrappingEnabled() { return mTextureWrapping; }
		TextureFilter getTextureFilter() { return mTextureFilter; }
	};
	
	class aprilExport RAMTexture : public Texture
	{
	protected:
		ImageSource* mBuffer;
	public:
		RAMTexture(chstr filename,bool dynamic);
		virtual ~RAMTexture();
		void load();
		void unload();
		bool isLoaded();
		Color getPixel(int x,int y);
		void setPixel(int x,int y,Color c);
		Color getInterpolatedPixel(float x,float y);
		int getSizeInBytes();
		
	};

	class aprilExport RenderSystem
	{
	protected:
		Window* mWindow;
		float mAlphaMultiplier;
		float mIdleUnloadTime;
		bool mDynamicLoading;
		TextureFilter mTextureFilter;
		bool mTextureWrapping;
		/*
		bool (*mUpdateCallback)(float);
		void (*mMouseDownCallback)(float,float,int);
		void (*mMouseUpCallback)(float,float,int);
		void (*mMouseMoveCallback)(float,float);
		void (*mKeyDownCallback)(unsigned int);
		void (*mKeyUpCallback)(unsigned int);
		void (*mCharCallback)(unsigned int);
		bool (*mQuitCallback)(bool);
		void (*mFocusCallback)(bool);
		*/
		gtypes::Matrix4 mModelviewMatrix,mProjectionMatrix;

		virtual void _setModelviewMatrix(const gtypes::Matrix4& matrix)=0;
		virtual void _setProjectionMatrix(const gtypes::Matrix4& matrix)=0;
	public:
		
		RenderSystem();
		virtual ~RenderSystem();

		virtual void assignWindow(Window* window)=0;

		// object creation
		hstr findTextureFile(chstr filename);
		virtual Texture* loadTexture(chstr filename,bool dynamic=false)=0;
		Texture* loadRAMTexture(chstr filename,bool dynamic=false);
		virtual Texture* createTextureFromMemory(unsigned char* rgba,int w,int h)=0;
		virtual Texture* createEmptyTexture(int w,int h,TextureFormat fmt=AT_XRGB,TextureType type=AT_NORMAL)=0;

		// modelview matrix transformation
		void setIdentityTransform();
		void translate(float x,float y,float z=0);
		void rotate(float angle,float ax=0,float ay=0,float az=-1);
		void scale(float s);
		void scale(float sx,float sy,float sz);
		// camera functions
		void lookAt(const gtypes::Vector3 &eye, const gtypes::Vector3 &direction, const gtypes::Vector3 &up);
		// projection matrix tronsformation
		void setOrthoProjection(float x,float y,float w,float h);
		void setOrthoProjection(gvec2 size);
		void setOrthoProjection(grect rect);
		void setPerspective(float fov, float aspect, float nearClip, float farClip);
		// rendersys matrix operations
		void setModelviewMatrix(const gtypes::Matrix4& matrix);
		void setProjectionMatrix(const gtypes::Matrix4& matrix);
		virtual bool isFullscreen() { return false; } //2DO - implement in derived classes
		virtual void setFullscreen(bool fullscreen) { } //2DO - implement in derived classes
		virtual void setResolution(int w, int h);
		
		const gtypes::Matrix4& getModelviewMatrix();
		const gtypes::Matrix4& getProjectionMatrix();
		// render state
		virtual void setBlendMode(BlendMode mode)=0;
		// caps
		virtual float getPixelOffset()=0;
		virtual hstr getName()=0;
		// rendering
		virtual void clear(bool color=true,bool depth=false)=0;
		virtual void setTexture(Texture* t)=0;
		virtual void render(RenderOp renderOp,TexturedVertex* v,int nVertices)=0;
		virtual void render(RenderOp renderOp,ColoredTexturedVertex* v,int nVertices)=0;
		virtual void render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)=0;
		virtual void render(RenderOp renderOp,PlainVertex* v,int nVertices)=0;
		virtual void render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)=0;
		virtual void render(RenderOp renderOp,ColoredVertex* v,int nVertices)=0;
		
		virtual void setRenderTarget(Texture* source)=0;
		
		void drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a=1);
		void drawTexturedQuad(float x,float y,float w,float h,float sx,float sy,float sw,float sh);
		void drawTexturedQuad(float x,float y,float w,float h,float sx,float sy,float sw,float sh,float r,float g,float b,float a);

		
		float getIdleTextureUnloadTime() { return mIdleUnloadTime; }
		void setIdleTextureUnloadTime(float time) { mIdleUnloadTime=time; }

		void logMessagef(chstr message, ...);
		void logMessage(chstr message,chstr prefix="[april] ");

		virtual void setAlphaMultiplier(float value)=0;
		float getAlphaMultiplier() { return mAlphaMultiplier; }


		virtual void beginFrame()=0;

		void registerUpdateCallback(bool (*callback)(float)) DEPRECATED_ATTRIBUTE;
		void registerMouseCallbacks(void (*mouse_dn)(float,float,int),
									void (*mouse_up)(float,float,int),
									void (*mouse_move)(float,float)) DEPRECATED_ATTRIBUTE;
		void registerKeyboardCallbacks(void (*key_dn)(unsigned int),
									   void (*key_up)(unsigned int),
									   void (*char_callback)(unsigned int)) DEPRECATED_ATTRIBUTE;
		void registerQuitCallback(bool (*quit_callback)(bool)) DEPRECATED_ATTRIBUTE;
		void registerWindowFocusCallback(void (*focus_callback)(bool)) DEPRECATED_ATTRIBUTE;

		void forceDynamicLoading(bool value) { mDynamicLoading=value; }
		bool isDynamicLoadingForced() { return mDynamicLoading; }
		
		Window* getWindow() { return mWindow; }
		
        virtual ImageSource* grabScreenshot()=0;
        
		virtual void enterMainLoop() DEPRECATED_ATTRIBUTE;
		virtual void terminateMainLoop() DEPRECATED_ATTRIBUTE;
		virtual gvec2 getCursorPos() DEPRECATED_ATTRIBUTE;
		virtual int getWindowWidth() DEPRECATED_ATTRIBUTE;
		virtual int getWindowHeight() DEPRECATED_ATTRIBUTE;
		virtual void setWindowTitle(chstr title) DEPRECATED_ATTRIBUTE;
		virtual void presentFrame(); // DEPRECATED_ATTRIBUTE; -- not deprecated because directx has its own way of presenting stuff.
		virtual void showSystemCursor(bool visible) DEPRECATED_ATTRIBUTE;
		virtual harray<DisplayMode> getSupportedDisplayModes()=0;

	};

	aprilFnExport void setLogFunction(void (*fnptr)(chstr));
	aprilFnExport void init();
	aprilFnExport void createRenderSystem(chstr rendersystem_name);
	aprilFnExport void createRenderTarget(int w,int h,bool fullscreen,chstr title);
	aprilFnExport void destroy();
	aprilFnExport void addTextureExtension(chstr extension);
	
	// global rendersys shortcut variable
	aprilFnExport extern april::RenderSystem* rendersys;
}

#endif