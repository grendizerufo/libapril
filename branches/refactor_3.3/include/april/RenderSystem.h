/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic render system.

#ifndef APRIL_RENDER_SYSTEM_H
#define APRIL_RENDER_SYSTEM_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <gtypes/Matrix4.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>

#include "aprilExport.h"
#include "aprilUtil.h"
#include "Color.h"
#include "RenderState.h"
#include "Texture.h"

#include "Window.h" // can be removed later

namespace april
{
	class Image;
	class PixelShader;
	class RamTexture;
	class Texture;
	class VertexShader;
	class Window;

	class aprilExport RenderSystem
	{
	public:
		friend class Texture;
		friend class Window;

		struct aprilExport DisplayMode
		{
		public:
			int width;
			int height;
			int refreshRate;

			DisplayMode(int width, int height, int refreshRate);
			~DisplayMode();

			bool operator==(const DisplayMode& other) const;
			bool operator!=(const DisplayMode& other) const;

			hstr toString();

		};
	
		struct aprilExport Options
		{
		public:
			bool depthBuffer;

			Options();
			~Options();

			hstr toString();

		};

		RenderSystem();
		virtual ~RenderSystem();
		virtual bool create(Options options);
		virtual bool destroy();

		virtual void assignWindow(Window* window) = 0;
		virtual void reset();

		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(Options, options, Options);
		HL_DEFINE_GET(harray<Texture*>, textures, Textures);
		HL_DEFINE_GET(grect, viewport, Viewport);
		HL_DEFINE_GET(gmat4, modelviewMatrix, ModelviewMatrix);
		void setModelviewMatrix(gmat4 matrix);
		HL_DEFINE_GET(gmat4, projectionMatrix, ProjectionMatrix);
		void setProjectionMatrix(gmat4 matrix);
		HL_DEFINE_GET(grect, orthoProjection, OrthoProjection);
		void setOrthoProjection(grect rect);
		void setOrthoProjection(gvec2 size);

		virtual harray<DisplayMode> getSupportedDisplayModes();
		virtual void setViewport(grect value);

		virtual float getPixelOffset() = 0;
		virtual int getMaxTextureSize() = 0;

		virtual void setTextureBlendMode(BlendMode blendMode) = 0;
		virtual void setTextureColorMode(ColorMode colorMode, unsigned char alpha = 255) = 0;
		virtual void setTextureFilter(Texture::Filter textureFilter) = 0;
		virtual void setTextureAddressMode(Texture::AddressMode textureAddressMode) = 0;
		virtual void setTexture(Texture* texture) = 0;
		virtual Texture* getRenderTarget() = 0;
		virtual void setRenderTarget(Texture* texture) = 0;
		virtual void setVertexShader(VertexShader* vertexShader) = 0;
		virtual void setPixelShader(PixelShader* pixelShader) = 0;

		Texture* createTexture(chstr filename, bool loadImmediately = true);
		Texture* createTexture(int w, int h, unsigned char* rgba);
		Texture* createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);
		Texture* createRamTexture(chstr filename, bool loadImmediately = true); // TODO - will be removed in a future version
		virtual PixelShader* createPixelShader() = 0;
		virtual PixelShader* createPixelShader(chstr filename) = 0;
		virtual VertexShader* createVertexShader() = 0;
		virtual VertexShader* createVertexShader(chstr filename) = 0;

		void setIdentityTransform();
		void translate(float x, float y, float z = 0.0f);
		void rotate(float angle, float ax = 0.0f, float ay = 0.0f, float az = -1.0f);
		void scale(float s);
		void scale(float sx, float sy, float sz);
		void lookAt(const gvec3 &eye, const gvec3 &direction, const gvec3 &up);
		void setPerspective(float fov, float aspect, float nearClip, float farClip);

		virtual void clear(bool useColor = true, bool depth = false) = 0;
		virtual void clear(bool depth, grect rect, Color color = Color::Clear) = 0;
		virtual void render(RenderOperation renderOperation, PlainVertex* v, int nVertices) = 0;
		virtual void render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color) = 0;
		virtual void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices) = 0;
		virtual void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color) = 0;
		virtual void render(RenderOperation renderOperation, ColoredVertex* v, int nVertices) = 0;
		virtual void render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices) = 0;
		
		void drawRect(grect rect, Color color);
		void drawFilledRect(grect rect, Color color);
		void drawTexturedRect(grect rect, grect src);
		void drawTexturedRect(grect rect, grect src, Color color);

		hstr findTextureFilename(chstr filename);
		void unloadTextures();
		virtual Image* takeScreenshot(int bpp = 3) = 0;
		virtual void presentFrame();

		DEPRECATED_ATTRIBUTE Texture* loadTexture(chstr filename, bool delayLoad = false) { return this->createTexture(filename, !delayLoad); }
		DEPRECATED_ATTRIBUTE Texture* loadRamTexture(chstr filename, bool delayLoad = false) { return this->createRamTexture(filename, !delayLoad); }

	protected:
		hstr name;
		bool created;
		Options options;
		harray<Texture*> textures;
		grect viewport;
		RenderState* state;
		Texture::Filter textureFilter;
		Texture::AddressMode textureAddressMode;
		gmat4 modelviewMatrix;
		gmat4 projectionMatrix;
		grect orthoProjection;

		virtual Texture* _createTexture(chstr filename) = 0;
		virtual Texture* _createTexture(int w, int h, unsigned char* rgba) = 0;
		virtual Texture* _createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear) = 0;

		void _registerTexture(Texture* texture);
		void _unregisterTexture(Texture* texture);

		virtual void _setModelviewMatrix(const gmat4& matrix) = 0;
		virtual void _setProjectionMatrix(const gmat4& matrix) = 0;
		
		virtual void _setResolution(int w, int h, bool fullscreen) = 0; // TODO - main part should be in window class

		unsigned int _numPrimitives(RenderOperation renderOperation, int nVertices);
		unsigned int _limitPrimitives(RenderOperation renderOperation, int nVertices);

	};

	// global rendersys shortcut variable
	aprilFnExport extern april::RenderSystem* rendersys;
	
}

#endif