/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Ivan Vucica                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>
#include <algorithm>

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>
#include <gtypes/Rectangle.h>

#if defined(__APPLE__) && !defined(_OPENGL)
#define _OPENGL
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif


#ifdef USE_IL
#include <IL/il.h>
#endif

#include "RenderSystem.h"
#ifdef _OPENGL
#include "RenderSystem_GL.h"
#else
#include "RenderSystem_DirectX9.h"
#endif
#include "ImageSource.h"
#include "Window.h"

april::RenderSystem* rendersys DEPRECATED_ATTRIBUTE = 0;

namespace april
{
	april::RenderSystem* rendersys;
	harray<hstr> extensions;

	void april_writelog(chstr message)
	{
		printf("%s\n",message.c_str());		
	}
	
	void (*g_logFunction)(chstr)=april_writelog;
	
	int hexstr_to_int(chstr s)
	{
		int i;
		sscanf(s.c_str(),"%x",&i);
		return i;
	}
	
	void hexstr_to_argb(chstr hex,unsigned char* a,unsigned char* r,unsigned char* g,unsigned char* b)
	{
		hstr value = hex;
		if (value(0,2) != "0x") value="0x"+value;
		if (value.size() == 8)
		{
			*r=hexstr_to_int(value(2,2));
			*g=hexstr_to_int(value(4,2));
			*b=hexstr_to_int(value(6,2));
			*a=255;
		}
		else if (value.size() == 10)
		{
			*a=hexstr_to_int(value(2,2));
			*r=hexstr_to_int(value(4,2));
			*g=hexstr_to_int(value(6,2));
			*b=hexstr_to_int(value(8,2));
		}
		else throw "Color format must be either 0xAARRGGBB or 0xRRGGBB";
	}
/*****************************************************************************************/	
	void PlainVertex::operator=(const gtypes::Vector3& v)
	{
		this->x=v.x;
		this->y=v.y;
		this->z=v.z;
	}
/*****************************************************************************************/
	Color::Color(float _a,float _r,float _g,float _b)
	{
		setColor(_a,_r,_g,_b);
	}

	Color::Color(int _a,int _r,int _g,int _b)
	{
		setColor(_a,_r,_g,_b);
	}

	Color::Color(unsigned char _a,unsigned char _r,unsigned char _g,unsigned char _b)
	{
		setColor(_a,_r,_g,_b);
	}

	Color::Color(unsigned int color)
	{
		setColor(color);
	}

	Color::Color(chstr hex)
	{
		setColor(hex);
	}

	Color::Color()
	{
		a=r=g=b=255;
	}

	void Color::setColor(float _a,float _r,float _g,float _b)
	{
		this->a=(unsigned char)(_a*255); this->r=(unsigned char)(_r*255);
		this->g=(unsigned char)(_g*255); this->b=(unsigned char)(_b*255);
	}

	void Color::setColor(int _a,int _r,int _g,int _b)
	{
		this->a=(unsigned char)_a; this->r=(unsigned char)_r;
		this->g=(unsigned char)_g; this->b=(unsigned char)_b;
	}

	void Color::setColor(unsigned char _a,unsigned char _r,unsigned char _g,unsigned char _b)
	{
		this->a=_a; this->r=_r; this->g=_g; this->b=_b;
	}

	void Color::setColor(unsigned int color)
	{
		this->a=color/256/256/256; this->r=color/256/256%256; this->g=color/256%256; this->b=color%256;
	}

	void Color::setColor(chstr hex)
	{
		// this is going to bite me in the arse on a little endian system...
		hexstr_to_argb(hex,&a,&r,&g,&b);
	}

	bool Color::operator==(Color& other)
	{
		return (this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a);
	}

	bool Color::operator!=(Color& other)
	{
		return !(*this == other);
	}
	
	Color Color::operator+(Color& other)
	{
		Color result(*this);
		result += other;
		return result;
	}

	Color Color::operator-(Color& other)
	{
		Color result(*this);
		result -= other;
		return result;
	}

	Color Color::operator*(Color& other)
	{
		Color result(*this);
		result *= other;
		return result;
	}

	Color Color::operator/(Color& other)
	{
		Color result(*this);
		result /= other;
		return result;
	}

	Color Color::operator*(float value)
	{
		Color result(*this);
		result *= value;
		return result;
	}

	Color Color::operator/(float value)
	{
		Color result(*this);
		result /= value;
		return result;
	}

	void Color::operator+=(Color& other)
	{
		this->r = hclamp(this->r + other.r, 0, 255);
		this->g = hclamp(this->g + other.g, 0, 255);
		this->b = hclamp(this->b + other.b, 0, 255);
		this->a = hclamp(this->a + other.a, 0, 255);
	}

	void Color::operator-=(Color& other)
	{
		this->r = hclamp(this->r - other.r, 0, 255);
		this->g = hclamp(this->g - other.g, 0, 255);
		this->b = hclamp(this->b - other.b, 0, 255);
		this->a = hclamp(this->a - other.a, 0, 255);
	}

	void Color::operator*=(Color& other)
	{
		this->r = hclamp((int)(this->r_float() * other.r), 0, 255);
		this->g = hclamp((int)(this->g_float() * other.g), 0, 255);
		this->b = hclamp((int)(this->b_float() * other.b), 0, 255);
		this->a = hclamp((int)(this->a_float() * other.a), 0, 255);
	}

	void Color::operator/=(Color& other)
	{
		this->r = hclamp((int)(this->r_float() / other.r), 0, 255);
		this->g = hclamp((int)(this->g_float() / other.g), 0, 255);
		this->b = hclamp((int)(this->b_float() / other.b), 0, 255);
		this->a = hclamp((int)(this->a_float() / other.a), 0, 255);
	}

	void Color::operator*=(float value)
	{
		this->r = hclamp((int)(this->r * value), 0, 255);
		this->g = hclamp((int)(this->g * value), 0, 255);
		this->b = hclamp((int)(this->b * value), 0, 255);
		this->a = hclamp((int)(this->a * value), 0, 255);
	}

	void Color::operator/=(float value)
	{
		this->r = hclamp((int)(this->r / value), 0, 255);
		this->g = hclamp((int)(this->g / value), 0, 255);
		this->b = hclamp((int)(this->b / value), 0, 255);
		this->a = hclamp((int)(this->a / value), 0, 255);
	}

/*****************************************************************************************/
	Texture::Texture()
	{
		mFilename="";
		mUnusedTimer=0;
		mTextureFilter=Linear;
		mTextureWrapping=1;
	}

	Texture::~Texture()
	{
		foreach (Texture*, it, mDynamicLinks)
			(*it)->removeDynamicLink(this);
	}
	
	Color Texture::getPixel(int x,int y)
	{
		return Color(0,0,0,0);
	}
	
	Color Texture::getInterpolatedPixel(float x,float y)
	{
		return Color(0,0,0,0);
	}
	
	void Texture::update(float time_increase)
	{
		if (mDynamic && isLoaded())
		{
			float max_time=rendersys->getIdleTextureUnloadTime();
			if (max_time > 0)
			{
				if (mUnusedTimer > max_time) unload();
				mUnusedTimer+=time_increase;
			}
		}
	}
	
	void Texture::addDynamicLink(Texture* lnk)
	{
		if (mDynamicLinks.contains(lnk)) return;
		mDynamicLinks+=lnk;
		lnk->addDynamicLink(this);
	}
	
	void Texture::removeDynamicLink(Texture* lnk)
	{
		if (mDynamicLinks.contains(lnk))
			mDynamicLinks-=lnk;
	}
	
	void  Texture::_resetUnusedTimer(bool recursive)
	{
		mUnusedTimer=0;
		if (recursive)
		{
			foreach (Texture*, it, mDynamicLinks)
				(*it)->_resetUnusedTimer(0);
		}
	}
/*****************************************************************************************/
	RAMTexture::RAMTexture(chstr filename,bool dynamic)
	{
		mFilename=filename;
		mBuffer=0;
		if (!dynamic) load();
	}


	RAMTexture::~RAMTexture()
	{
		unload();
	}
	
	void RAMTexture::load()
	{
		if (!mBuffer)
		{
			rendersys->logMessage("loading RAM texture '"+mFilename+"'");
			mBuffer=loadImage(mFilename);
			mWidth=mBuffer->w;
			mHeight=mBuffer->h;
		}
	}
	
	void RAMTexture::unload()
	{
		if (mBuffer)
		{
			rendersys->logMessage("unloading RAM texture '"+mFilename+"'");
			delete mBuffer;
			mBuffer=0;
		}
	}
	
	bool RAMTexture::isLoaded()
	{
		return mBuffer != 0;
	}
	
	Color RAMTexture::getPixel(int x,int y)
	{
		if (!mBuffer) load();
		mUnusedTimer=0;
		return mBuffer->getPixel(x,y);
	}
	
	void RAMTexture::setPixel(int x,int y,Color c)
	{
		if (!mBuffer) load();
		mUnusedTimer=0;
		mBuffer->setPixel(x,y,c);
	}
	
	Color RAMTexture::getInterpolatedPixel(float x,float y)
	{
		if (!mBuffer) load();
		mUnusedTimer=0;
		return mBuffer->getInterpolatedPixel(x,y);
	}
	
	int RAMTexture::getSizeInBytes()
	{
		return 0;
	}
/*****************************************************************************************/
	RenderSystem::RenderSystem()
	{
		mAlphaMultiplier=1.0f;
		mDynamicLoading=0;
		mIdleUnloadTime=0;
		mTextureFilter=Linear;
		mTextureWrapping=1;
	}
	
	RenderSystem::~RenderSystem()
	{
		
	}

	void RenderSystem::drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a)
	{
		PlainVertex v[4];
		v[0].x=x;   v[0].y=y;   v[0].z=0;
		v[1].x=x+w; v[1].y=y;   v[1].z=0;
		v[2].x=x;   v[2].y=y+h; v[2].z=0;
		v[3].x=x+w; v[3].y=y+h; v[3].z=0;
		
		render(TriangleStrip,v,4,r,g,b,a);
	}
	
	void RenderSystem::drawTexturedQuad(float x,float y,float w,float h,float sx,float sy,float sw,float sh)
	{
		TexturedVertex v[4];
		v[0].x=x;   v[0].y=y;   v[0].z=0; v[0].u=sx;    v[0].v=sy;
		v[1].x=x+w; v[1].y=y;   v[1].z=0; v[1].u=sx+sw; v[1].v=sy;
		v[2].x=x;   v[2].y=y+h; v[2].z=0; v[2].u=sx;    v[2].v=sy+sh;
		v[3].x=x+w; v[3].y=y+h; v[3].z=0; v[3].u=sx+sw; v[3].v=sy+sh;

		render(TriangleStrip,v,4);		
	}
	
	void RenderSystem::drawTexturedQuad(float x,float y,float w,float h,float sx,float sy,float sw,float sh,float r,float g,float b,float a)
	{
		TexturedVertex v[4];
		v[0].x=x;   v[0].y=y;   v[0].z=0; v[0].u=sx;    v[0].v=sy;
		v[1].x=x+w; v[1].y=y;   v[1].z=0; v[1].u=sx+sw; v[1].v=sy;
		v[2].x=x;   v[2].y=y+h; v[2].z=0; v[2].u=sx;    v[2].v=sy+sh;
		v[3].x=x+w; v[3].y=y+h; v[3].z=0; v[3].u=sx+sw; v[3].v=sy+sh;
		
		render(TriangleStrip,v,4,r,g,b,a);	
	}
	
	hstr RenderSystem::findTextureFile(chstr _filename)
	{
		hstr filename = _filename;
		if (hfile::exists(filename)) return filename;
		hstr name;
		foreach (hstr, it, extensions)
		{
			name=filename+(*it);
			if (hfile::exists(name)) return name;
		}
		if (filename.rfind(".") != -1) {
			filename = filename.substr(0, filename.rfind("."));
			foreach (hstr, it, extensions)
			{
				name=filename+(*it);
				if (hfile::exists(name)) return name;
			}
		}

		return "";
	}
	
	void RenderSystem::logMessagef(chstr message, ...)
	{
		va_list vl;
		va_start(vl, message);
		logMessage(hvsprintf(message.c_str(), vl));;
		va_end(vl);
	}
	
	void RenderSystem::logMessage(chstr message,chstr prefix)
	{
		g_logFunction(prefix+message);
	}
	
	void RenderSystem::registerUpdateCallback(bool (*callback)(float))
	{
		mWindow->setUpdateCallback(callback);
		
		logMessage("RenderSystem::registerUpdateCallback() is deprecated");
	}

	void RenderSystem::registerMouseCallbacks(void (*mouse_dn)(float,float,int),
								              void (*mouse_up)(float,float,int),
								              void (*mouse_move)(float,float))
	{
		mWindow->setMouseCallbacks(mouse_dn,mouse_up,mouse_move);
		
		logMessage("RenderSystem::registerMouseCallbacks() is deprecated");
	}
	
	void RenderSystem::registerKeyboardCallbacks(void (*key_dn)(unsigned int),
								                 void (*key_up)(unsigned int),
												 void (*char_callback)(unsigned int))
	{
		mWindow->setKeyboardCallbacks(key_dn, key_up, char_callback);
		logMessage("RenderSystem::registerKeyboardCallbacks() is deprecated");
	}
	
	void RenderSystem::registerQuitCallback(bool (*quit_callback)(bool can_reject))
	{
		mWindow->setQuitCallback(quit_callback);
		logMessage("RenderSystem::registerQuitCallback() is deprecated");
	}
	
	void RenderSystem::registerWindowFocusCallback(void (*focus_callback)(bool current_focus))
	{
		mWindow->setWindowFocusCallback(focus_callback);		
		logMessage("RenderSystem::registerFocusCallback() is deprecated");
	}
	
	Texture* RenderSystem::loadRAMTexture(chstr filename,bool dynamic)
	{
		hstr name=findTextureFile(filename);
		if (name=="") return 0;
		return new RAMTexture(name,dynamic);
	}
	
	void RenderSystem::setIdentityTransform()
	{
		mModelviewMatrix.setIdentity();
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::translate(float x,float y,float z)
	{
		mModelviewMatrix.translate(x,y,z);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::rotate(float angle,float ax,float ay,float az)
	{
		mModelviewMatrix.rotate(ax,ay,az,angle);
		_setModelviewMatrix(mModelviewMatrix);
	}	
	
	void RenderSystem::scale(float s)
	{
		mModelviewMatrix.scale(s);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::scale(float sx,float sy,float sz)
	{
		mModelviewMatrix.scale(sx,sy,sz);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::lookAt(const gtypes::Vector3 &eye, const gtypes::Vector3 &direction, const gtypes::Vector3 &up)
	{
		mModelviewMatrix.lookAt(eye,direction,up);
		_setModelviewMatrix(mModelviewMatrix);
	}
		
	void RenderSystem::setOrthoProjection(float x, float y, float w, float h)
	{
		setOrthoProjection(grect(x, y, w, h));
	}
	
	void RenderSystem::setOrthoProjection(gvec2 size)
	{
		setOrthoProjection(grect(0.0f, 0.0f, size));
	}
	
	void RenderSystem::setOrthoProjection(grect rect)
	{
		float t = getPixelOffset();
		float wnd_w = (float)mWindow->getWindowWidth();
		float wnd_h = (float)mWindow->getWindowHeight();
		rect.x -= t * rect.w / wnd_w;
		rect.y -= t * rect.h / wnd_h;
		mProjectionMatrix.ortho(rect);
		_setProjectionMatrix(mProjectionMatrix);
	}
	
    void RenderSystem::setPerspective(float fov, float aspect, float nearClip, float farClip)
	{
		mProjectionMatrix.perspective(fov,aspect,nearClip,farClip);
		_setProjectionMatrix(mProjectionMatrix);
	}
	
	void RenderSystem::setModelviewMatrix(const gtypes::Matrix4& matrix)
	{
		mModelviewMatrix=matrix;
		_setModelviewMatrix(matrix);
	}
	void RenderSystem::setProjectionMatrix(const gtypes::Matrix4& matrix)
	{
		mProjectionMatrix=matrix;
		_setProjectionMatrix(matrix);
	}
	
	void RenderSystem::setResolution(int w, int h)
	{
		logMessage(hsprintf("changing resolution: %d x %d", w, h));
		mWindow->_setResolution(w, h);
	}

	const gtypes::Matrix4& RenderSystem::getModelviewMatrix()
	{
		return mModelviewMatrix;
	}
	
	const gtypes::Matrix4& RenderSystem::getProjectionMatrix()
	{
		return mProjectionMatrix;
	}
	
/*********************************************************************************/

/* deprecated funcs	*/

	int RenderSystem::getWindowWidth() 
	{ 
		return getWindow()->getWindowWidth(); 
	}
	
	int RenderSystem::getWindowHeight() 
	{ 
		return getWindow()->getWindowHeight(); 
	}
	
	void RenderSystem::enterMainLoop()
	{
		logMessage("RenderSystem::enterMainLoop() is deprecated"); 
		getWindow()->enterMainLoop(); 
	}

	void RenderSystem::terminateMainLoop()
	{
		logMessage("RenderSystem::enterMainLoop() is deprecated"); 
		getWindow()->terminateMainLoop(); 
	}
	gvec2 RenderSystem::getCursorPos()
	{
		return getWindow()->getCursorPos();
	}
	
	void RenderSystem::setWindowTitle(chstr title)
	{
		static bool done=0;
		
		if (!done) { done=1; logMessage("RenderSystem::setWindowTitle() is deprecated"); }
		getWindow()->setWindowTitle(title);
	}
	void RenderSystem::presentFrame()
	{
		getWindow()->presentFrame();
	}
	void RenderSystem::showSystemCursor(bool visible)
	{
		getWindow()->showSystemCursor(visible);
	}
	
/*********************************************************************************/
	void init()
	{
#ifdef USE_IL
		ilInit();
#endif
		extensions+=".png";
		extensions+=".jpg";
#if TARGET_OS_IPHONE
		extensions+=".pvr";
#endif
	}
	
	void createRenderSystem(chstr rendersystem_name)
	{
#ifdef _OPENGL
		createGLRenderSystem();
#else
		createDX9RenderSystem();
#endif
	}
	
	void createRenderTarget(int w,int h,bool fullscreen,chstr title)
	{
#ifdef _WIN32
		Window* window = createAprilWindow("Win32", w, h, fullscreen, title);
#else
		Window* window = createAprilWindow("SDL", w, h, fullscreen, title);
#endif
		april::rendersys->assignWindow(window);
	}
	
	void setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction=fnptr;
	}
	
	void destroy()
	{
		if (!rendersys) {
			return;
		}
		delete rendersys->getWindow();
		delete rendersys;
		rendersys = 0;
	}
	
	void addTextureExtension(chstr extension)
	{
		extensions+=extension;
	}

}