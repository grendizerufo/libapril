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
/// Defines a DirectX render system.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_RENDER_SYSTEM_H
#define APRIL_DIRECTX9_RENDER_SYSTEM_H

#include "RenderSystem.h"

struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DSurface9;

namespace april
{
	class DirectX9_PixelShader;
	class DirectX9_Texture;
	class DirectX9_VertexShader;
	class Window;

	class DirectX9_RenderSystem : public RenderSystem
	{
	public:
		friend class DirectX9_PixelShader;
		friend class DirectX9_Texture;
		friend class DirectX9_VertexShader;

		DirectX9_RenderSystem();
		~DirectX9_RenderSystem();
		bool create(chstr options);
		bool destroy();

		//void reset();
		void assignWindow(Window* window);

		float getPixelOffset() { return 0.5f; }
		grect getViewport();
		void setViewport(grect rect);
		void setBlendMode(BlendMode mode);
		void setColorMode(ColorMode mode, unsigned char alpha = 255);
		void setTextureFilter(Texture::Filter textureFilter);
		void setTextureAddressMode(Texture::AddressMode textureAddressMode);
		harray<DisplayMode> getSupportedDisplayModes();
		void setTexture(Texture* texture);
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

		void setResolution(int w, int h);

		Texture* loadTexture(chstr filename, bool dynamic);
		Texture* createTexture(int w, int h, unsigned char* rgba);
		Texture* createTexture(int w, int h, Texture::Format format = Texture::FORMAT_RGBA, Texture::Type type = Texture::TYPE_NORMAL, Color color = APRIL_COLOR_CLEAR);
		PixelShader* createPixelShader();
		PixelShader* createPixelShader(chstr filename);
		VertexShader* createVertexShader();
		VertexShader* createVertexShader(chstr filename);

		void clear(bool useColor = true, bool depth = false);
		void clear(bool depth, grect rect, Color color = APRIL_COLOR_CLEAR);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);

		ImageSource* takeScreenshot(int bpp = 3);
		void presentFrame();

	protected:
		bool zBufferEnabled;
		bool textureCoordinatesEnabled;
		bool colorEnabled;
		IDirect3D9* d3d;
		IDirect3DDevice9* d3dDevice;
		DirectX9_Texture* activeTexture;
		DirectX9_Texture* renderTarget;
		IDirect3DSurface9* backBuffer;
		harray<DisplayMode> supportedDisplayModes;

		void _configureDevice();

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	};

}
#endif
#endif
