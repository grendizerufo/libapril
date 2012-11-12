/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <d3d11_1.h>
#include <stdio.h>

#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX11_PixelShader.h"
#include "DirectX11_RenderSystem.h"
#include "DirectX11_Texture.h"
#include "DirectX11_VertexShader.h"
#include "ImageSource.h"
#include "Keys.h"
#include "Platform.h"
#include "Timer.h"
#include "WinRT_Window.h"

using namespace Microsoft::WRL;

#define PLAIN_FVF (D3DFVF_XYZ)
#define COLOR_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define TEX_FVF (D3DFVF_XYZ | D3DFVF_TEX1)
#define TEX_COLOR_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE)
#define TEX_COLOR_TONE_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR)
#define VERTICES_BUFFER_COUNT 8192
#define UINT_RGBA_TO_ARGB(c) ((((c) >> 8) & 0xFFFFFF) | (((c) & 0xFF) << 24))

namespace april
{
	static const char* defaultVertexShaderCode = 
		"void VS(in float4 posIn : POSITION, out float4 posOut : SV_Position)\n"
		"{\n"
		"    posOut = posIn;\n"
		"}\n";
	
	static const char* defaultPixelShaderCode = 
		"void PS(out float4 colorOut : SV_Target)\n"
		"{\n"
		"    colorOut = float4(1.0f, 1.0f, 1.0f, 1.0f);\n"
		"}\n";

	// TODO - refactor
	harray<DirectX11_Texture*> gRenderTargets;

	// TODO - refactor
	int DirectX11_RenderSystem::_getMaxTextureSize()
	{
		// TODO
		/*
		if (this->d3dDevice == NULL)
		{
			return 0;
		}
		D3DCAPS9 caps;
		this->d3dDevice->GetDeviceCaps(&caps);
		return caps.MaxTextureWidth;
		*/
		return 1024;
	}

	D3D11_PRIMITIVE_TOPOLOGY dx11_render_ops[]=
	{
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// ROP_TRIANGLE_LIST
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// ROP_TRIANGLE_STRIP
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,		// triangle fans are deprecated in DX11
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,		// ROP_LINE_LIST
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,		// ROP_LINE_STRIP
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,		// ROP_POINT_LIST
	};

	ColoredTexturedVertex static_ctv[VERTICES_BUFFER_COUNT];
	ColoredVertex static_cv[VERTICES_BUFFER_COUNT];

	unsigned int _numPrimitives(RenderOp renderOp, int nVertices)
	{
		switch (renderOp)
		{
		case TriangleList:
			return nVertices / 3;
		case TriangleStrip:
			return nVertices - 2;
		case TriangleFan:
			return nVertices - 1;
		case LineList:
			return nVertices / 2;
		case LineStrip:
			return nVertices - 1;
		case PointList:
			return nVertices;
		}
		return 0;
	}
	
	DirectX11_RenderSystem::DirectX11_RenderSystem() : RenderSystem(), zBufferEnabled(false),
		textureCoordinatesEnabled(false), colorEnabled(false), activeTexture(NULL), activeVertexShader(NULL),
		activePixelShader(NULL), renderTarget(NULL), defaultVertexShader(NULL), defaultPixelShader(NULL)
	{
		this->name = APRIL_RS_DIRECTX11;
		this->d3dDevice = nullptr;
		this->d3dDeviceContext = nullptr;
		this->swapChain = nullptr;
		this->indexBuffer = nullptr;
	}

	DirectX11_RenderSystem::~DirectX11_RenderSystem()
	{
		this->destroy();
	}

	bool DirectX11_RenderSystem::create(chstr options)
	{
		if (!RenderSystem::create(options))
		{
			return false;
		}
		this->zBufferEnabled = options.contains("zbuffer");
		this->textureCoordinatesEnabled = false;
		this->colorEnabled = false;
		this->activeTexture = NULL;
		this->activeVertexShader = NULL;
		this->activePixelShader = NULL;
		this->renderTarget = NULL;
		this->renderTargetView = nullptr;
		return true;
	}

	bool DirectX11_RenderSystem::destroy()
	{
		if (!RenderSystem::destroy())
		{
			return false;
		}
		if (this->defaultVertexShader != NULL)
		{
			delete this->defaultVertexShader;
			this->defaultVertexShader = NULL;
		}
		if (this->defaultPixelShader != NULL)
		{
			delete this->defaultPixelShader;
			this->defaultPixelShader = NULL;
		}
		this->indexBuffer = nullptr;
		return true;
	}

	void DirectX11_RenderSystem::assignWindow(Window* window)
	{
		unsigned int creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_1
		};

		ComPtr<ID3D11Device> _d3dDevice;
		ComPtr<ID3D11DeviceContext> _d3dDeviceContext;
		HRESULT hr;
		hr = D3D11CreateDevice(
			nullptr,                    // specify nullptr to use the default adapter
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,                    // leave as nullptr if hardware is used
			creationFlags,              // optionally set debug and Direct2D compatibility flags
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,          // always set this to D3D11_SDK_VERSION
			&_d3dDevice,
			nullptr,
			&_d3dDeviceContext
		);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create DX11 device!");
		}
		hr = _d3dDevice.As(&this->d3dDevice);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve Direct3D 11.1 device interface!");
		}
		hr = _d3dDeviceContext.As(&this->d3dDeviceContext);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve Direct3D 11.1 device context interface!");
		}
		// device config
		this->_configureDevice();
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), nullptr);
		if (this->genericIndices.size() == 0)
		{
			for_iter (i, 0, 65536)
			{
				this->genericIndices += (unsigned short)i;
			}
		}
		D3D11_BUFFER_DESC indexBufferDescription;
		indexBufferDescription.ByteWidth = sizeof(unsigned short) * this->genericIndices.size();
		indexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDescription.CPUAccessFlags = 0;
		indexBufferDescription.MiscFlags = 0;
		indexBufferDescription.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA indexBufferData;
		indexBufferData.pSysMem = &this->genericIndices.first();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

        if (this->indexBuffer == nullptr)
		{
			hr = this->d3dDevice->CreateBuffer(&indexBufferDescription, &indexBufferData, &this->indexBuffer);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to create generic index buffer!");
			}
		}
		this->clear(true, false);
		this->presentFrame();
		this->orthoProjection.setSize((float)window->getWidth(), (float)window->getHeight());
		if (this->defaultVertexShader == NULL)
		{
			this->defaultVertexShader = this->createVertexShader();
			this->defaultVertexShader->compile(defaultVertexShaderCode);
		}
		if (this->defaultPixelShader == NULL)
		{
			this->defaultPixelShader = this->createPixelShader();
			this->defaultPixelShader->compile(defaultPixelShaderCode);
		}
		this->setVertexShader(this->defaultVertexShader);
		this->setPixelShader(this->defaultPixelShader);
	}

	void DirectX11_RenderSystem::_createSwapChain(int width, int height)
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Stereo = false;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.Flags = 0;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		// Once the swap chain description is configured, it must be
		// created on the same adapter as the existing D3D Device.
		HRESULT hr;
		ComPtr<IDXGIDevice2> dxgiDevice;
		hr = this->d3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve DXGI device!");
		}
		hr = dxgiDevice->SetMaximumFrameLatency(1);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to set MaximumFrameLatency!");
		}
		ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(&dxgiAdapter);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get adapter from DXGI device!");
		}
		ComPtr<IDXGIFactory2> dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get parent factory from DXGI adapter!");
		}
		hr = dxgiFactory->CreateSwapChainForCoreWindow(this->d3dDevice.Get(),
			reinterpret_cast<IUnknown*>(april::WinRT::View->getCoreWindow()), &swapChainDesc, nullptr, &this->swapChain);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create swap chain!");
		}
	}

	void DirectX11_RenderSystem::_configureDevice()
	{
		HRESULT hr;
		if (this->swapChain != nullptr)
		{
			hr = this->swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to resize swap chain buffers!");
			}
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		ComPtr<ID3D11Texture2D> _backBuffer;
		hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get swap chain back buffer!");
		}
		hr = this->d3dDevice->CreateRenderTargetView(_backBuffer.Get(), nullptr, &this->renderTargetView);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create render target view!");
		}
		D3D11_TEXTURE2D_DESC backBufferDesc = {0};
		_backBuffer->GetDesc(&backBufferDesc);
		this->setViewport(grect(0.0f, 0.0f, (float)backBufferDesc.Width, (float)backBufferDesc.Height));
		/*
		// calls on init and device reset
		this->d3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
		this->d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		this->d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		// separate alpha blending to use proper alpha blending
		this->d3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, 1);
		this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
		this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
		// vertex color blending
		this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		this->setTextureFilter(this->textureFilter);
		*/
	}

	harray<DisplayMode> DirectX11_RenderSystem::getSupportedDisplayModes()
	{
		if (this->supportedDisplayModes.size() == 0)
		{
			gvec2 resolution = april::getSystemInfo().displayResolution;
			DisplayMode displayMode;
			displayMode.width = (int)resolution.x;
			displayMode.height = (int)resolution.y;
			displayMode.refreshRate = 60;
			this->supportedDisplayModes += displayMode;
		}
		return this->supportedDisplayModes;
	}

	grect DirectX11_RenderSystem::getViewport()
	{
		D3D11_VIEWPORT viewport;
		unsigned int count = 1;
		this->d3dDeviceContext->RSGetViewports(&count, &viewport);
		return grect((float)viewport.TopLeftX, (float)viewport.TopLeftY, (float)viewport.Width, (float)viewport.Height);
	}

	void DirectX11_RenderSystem::setViewport(grect rect)
	{
		D3D11_VIEWPORT viewport;
		viewport.MinDepth = D3D11_MIN_DEPTH;
		viewport.MaxDepth = D3D11_MAX_DEPTH;
		// these double-casts are to ensure consistent behavior among rendering systems
		viewport.TopLeftX = (float)(int)rect.x;
		viewport.TopLeftY = (float)(int)rect.y;
		viewport.Width = (float)(int)rect.w;
		viewport.Height = (float)(int)rect.h;
		this->d3dDeviceContext->RSSetViewports(1, &viewport);
	}

	void DirectX11_RenderSystem::setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO
		/*
		switch (textureBlendMode)
		{
		case DEFAULT:
		case ALPHA_BLEND:
			this->setPixelShader(this->defaultPixelShader);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case ADD:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case SUBTRACT:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case OVERWRITE:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture blend mode!");
			break;
		}
		*/
	}

	void DirectX11_RenderSystem::setTextureColorMode(ColorMode textureColorMode, unsigned char alpha)
	{
		// TODO
		/*
		switch (textureColorMode)
		{
		case NORMAL:
		case MULTIPLY:
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			break;
		case LERP:
			this->d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(alpha, alpha, alpha, alpha));
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			break;
		case ALPHA_MAP:
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture color mode!");
			break;
		}
		*/
	}

	void DirectX11_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		// TODO
		/*
		switch (textureFilter)
		{
		case Texture::FILTER_LINEAR:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			break;
		case Texture::FILTER_NEAREST:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture filter!");
			break;
		}
		this->textureFilter = textureFilter;
		*/
	}

	void DirectX11_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		// TODO
		/*
		switch (textureAddressMode)
		{
		case Texture::ADDRESS_WRAP:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			break;
		case Texture::ADDRESS_CLAMP:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture address mode!");
			break;
		}
		this->textureAddressMode = textureAddressMode;
		*/
	}

	void DirectX11_RenderSystem::setTexture(Texture* texture)
	{
		this->activeTexture = (DirectX11_Texture*)texture;
		if (this->activeTexture != NULL)
		{
			Texture::Filter filter = this->activeTexture->getFilter();
			if (this->textureFilter != filter)
			{
				this->setTextureFilter(filter);
			}
			Texture::AddressMode addressMode = this->activeTexture->getAddressMode();
			if (this->textureAddressMode != addressMode)
			{
				this->setTextureAddressMode(addressMode);
			}
			this->activeTexture->load();
			// TODO
			//this->d3dDevice->SetTexture(0, this->activeTexture->_getTexture());
		}
		else
		{
			// TODO
			//this->d3dDevice->SetTexture(0, NULL);
		}
	}

	Texture* DirectX11_RenderSystem::getRenderTarget()
	{
		return this->renderTarget;
	}
	
	void DirectX11_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODO
		/*
		if (this->renderTarget != NULL)
		{
			this->d3dDevice->EndScene();
		}
		DirectX11_Texture* texture = (DirectX11_Texture*)source;
		if (texture == NULL)
		{
			this->d3dDevice->SetRenderTarget(0, this->backBuffer);
		}
		else
		{
			this->d3dDevice->SetRenderTarget(0, texture->_getSurface());
		}
		this->renderTarget = texture;
		if (this->renderTarget != NULL)
		{
			this->d3dDevice->BeginScene();
		}
		*/
	}
	
	void DirectX11_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		this->activePixelShader = (DirectX11_PixelShader*)pixelShader;
	}

	void DirectX11_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		this->activeVertexShader = (DirectX11_VertexShader*)vertexShader;
	}

	void DirectX11_RenderSystem::_setPixelShader(DirectX11_PixelShader* pixelShader)
	{
		if (pixelShader != NULL)
		{
			this->d3dDeviceContext->PSSetShader(pixelShader->dx11Shader, NULL, 0);
		}
		else
		{
			this->d3dDeviceContext->PSSetShader(NULL, NULL, 0);
		}
	}

	void DirectX11_RenderSystem::_setVertexShader(DirectX11_VertexShader* vertexShader)
	{
		if (vertexShader != NULL)
		{
			this->d3dDeviceContext->VSSetShader(vertexShader->dx11Shader, NULL, 0);
			this->d3dDeviceContext->IASetInputLayout(vertexShader->inputLayout.Get());
		}
		else
		{
			this->d3dDeviceContext->VSSetShader(NULL, NULL, 0);
		}
	}

	void DirectX11_RenderSystem::setResolution(int w, int h)
	{
		RenderSystem::setResolution(w, h);
		hlog::writef(april::logTag, "Resetting device for %d x %d...", april::window->getWidth(), april::window->getHeight());
		this->_createSwapChain(w, h);
		this->_setModelviewMatrix(this->modelviewMatrix);
		this->_setProjectionMatrix(this->projectionMatrix);
	}

	Texture* DirectX11_RenderSystem::_createTexture(chstr filename)
	{
		return new DirectX11_Texture(filename);
	}

	Texture* DirectX11_RenderSystem::createTexture(int w, int h, unsigned char* rgba)
	{
		return new DirectX11_Texture(w, h, rgba);
	}
	
	Texture* DirectX11_RenderSystem::createTexture(int w, int h, Texture::Format format, Texture::Type type, Color color)
	{
		return new DirectX11_Texture(w, h, format, type, color);
	}
	
	PixelShader* DirectX11_RenderSystem::createPixelShader()
	{
		return new DirectX11_PixelShader();
	}

	PixelShader* DirectX11_RenderSystem::createPixelShader(chstr filename)
	{
		return new DirectX11_PixelShader(filename);
	}

	VertexShader* DirectX11_RenderSystem::createVertexShader()
	{
		return new DirectX11_VertexShader();
	}

	VertexShader* DirectX11_RenderSystem::createVertexShader(chstr filename)
	{
		return new DirectX11_VertexShader(filename);
	}

	void DirectX11_RenderSystem::clear(bool useColor, bool depth)
	{
		static const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}
	
	void DirectX11_RenderSystem::clear(bool depth, grect rect, Color color)
	{
		// TODO
		const float clearColor[4] = {color.b_f(), color.g_f(), color.r_f(), color.a_f()};
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}
	
	void DirectX11_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
	{
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		/*
        // Create an input layout that matches the layout defined in the vertex shader code.
        // For this lesson, this is simply a float2 vector defining the vertex position.
        const D3D11_INPUT_ELEMENT_DESC basicVertexLayoutDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        ComPtr<ID3D11InputLayout> inputLayout;
        this->d3dDevice->CreateInputLayout(basicVertexLayoutDesc, ARRAYSIZE(basicVertexLayoutDesc), vertexShaderBytecode->Data,
                vertexShaderBytecode->Length,
                &inputLayout
            );
			*/

		memset(&this->vertexBufferDescription, 0, sizeof(D3D11_BUFFER_DESC));
		this->vertexBufferDescription.ByteWidth = sizeof(PlainVertex) * nVertices;
		this->vertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
		this->vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		this->vertexBufferDescription.CPUAccessFlags = 0;
		this->vertexBufferDescription.MiscFlags = 0;
		this->vertexBufferDescription.StructureByteStride = 0;
		this->vertexBufferData.pSysMem = v;
		this->vertexBufferData.SysMemPitch = 0;
		this->vertexBufferData.SysMemSlicePitch = 0;
		ComPtr<ID3D11Buffer> vertexBuffer;
        unsigned int stride = sizeof(PlainVertex);
        unsigned int offset = 0;
		this->d3dDevice->CreateBuffer(&this->vertexBufferDescription, &this->vertexBufferData, &vertexBuffer);
		this->d3dDeviceContext->IASetPrimitiveTopology(dx11_render_ops[renderOp]);
        this->d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		D3D11_BUFFER_DESC indexBufferDescription;
		this->indexBuffer.Get()->GetDesc(&indexBufferDescription);
		// TODO - cannot render more than 64k at once
		indexBufferDescription.ByteWidth = sizeof(unsigned short) * hmin(nVertices, 65535);
		this->d3dDeviceContext->IASetIndexBuffer(this->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		this->_setPixelShader(this->activePixelShader);
		this->_setVertexShader(this->activeVertexShader);
		this->d3dDeviceContext->DrawIndexed(nVertices, 0, 0);

		// TODO
		/*
		this->d3dDevice->SetFVF(PLAIN_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx11_render_ops[renderOp], _numPrimitives(renderOp, nVertices), v, sizeof(PlainVertex));
		*/
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		this->render(renderOp, v, nVertices);
		// TODO
		/*
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		unsigned int colorDx9 = D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b);
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p = cv;
		for_iter (i, 0, nVertices)
		{
			p[i].x = v[i].x;
			p[i].y = v[i].y;
			p[i].z = v[i].z;
			p[i].color = colorDx9;
		}
		this->d3dDevice->SetFVF(COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx11_render_ops[renderOp], _numPrimitives(renderOp, nVertices), cv, sizeof(ColoredVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
		*/
	}
	
	void DirectX11_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
	{
		// TODO
		/*
		this->d3dDevice->SetFVF(TEX_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx11_render_ops[renderOp], _numPrimitives(renderOp, nVertices), v, sizeof(TexturedVertex));
		*/
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		// TODO
		/*
		unsigned int colorDx9 = D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b);
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		ColoredTexturedVertex* p = ctv;
		for_iter (i, 0, nVertices)
		{
			p[i].x = v[i].x;
			p[i].y = v[i].y;
			p[i].z = v[i].z;
			p[i].u = v[i].u;
			p[i].v = v[i].v;
			p[i].color = colorDx9;
		}
		this->d3dDevice->SetFVF(TEX_COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx11_render_ops[renderOp], _numPrimitives(renderOp, nVertices), ctv, sizeof(ColoredTexturedVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
		*/
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
	{
		// TODO
		/*
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p = cv;
		for_iter (i, 0, nVertices)
		{
			p[i].x = v[i].x;
			p[i].y = v[i].y;
			p[i].z = v[i].z;
			p[i].color = UINT_RGBA_TO_ARGB(v[i].color);
		}
		this->d3dDevice->SetFVF(COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx11_render_ops[renderOp], _numPrimitives(renderOp, nVertices), cv, sizeof(ColoredVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
		*/
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
	{
		// TODO
		/*
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		ColoredTexturedVertex* p = ctv;
		for_iter (i, 0, nVertices)
		{
			p[i].x = v[i].x;
			p[i].y = v[i].y;
			p[i].z = v[i].z;
			p[i].u = v[i].u;
			p[i].v = v[i].v;
			p[i].color = UINT_RGBA_TO_ARGB(v[i].color);
		}
		this->d3dDevice->SetFVF(TEX_COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx11_render_ops[renderOp], _numPrimitives(renderOp, nVertices), ctv, sizeof(ColoredTexturedVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
		*/
	}

	void DirectX11_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		// TODO
		/*
		this->d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)matrix.data);
		*/
	}

	void DirectX11_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		// TODO
		/*
		this->d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)matrix.data);
		*/
	}

	ImageSource* DirectX11_RenderSystem::takeScreenshot(int bpp)
	{
		// TODO
		/*
#ifdef _DEBUG
		hlog::write(april::logTag, "Grabbing screenshot...");
#endif
		D3DSURFACE_DESC desc;
		this->backBuffer->GetDesc(&desc);
		if (desc.Format != D3DFMT_X8R8G8B8)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, backbuffer format not supported, expected X8R8G8B8, got: " + hstr(desc.Format));
			return NULL;
		}
		IDirect3DSurface9* buffer;
		HRESULT hr = this->d3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &buffer, NULL);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, CreateOffscreenPlainSurface() call failed.");
			return NULL;
		}
		hr = this->d3dDevice->GetRenderTargetData(this->backBuffer, buffer);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, GetRenderTargetData() call failed.");
			buffer->Release();
			return NULL;
		}		
		D3DLOCKED_RECT rect;
		hr = buffer->LockRect(&rect, NULL, D3DLOCK_DONOTWAIT);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, surface lock failed.");
			buffer->Release();
			return NULL;
		}
		
		ImageSource* img = new ImageSource();
		img->w = desc.Width;
		img->h = desc.Height;
		img->bpp = bpp;
		img->format = (bpp == 4 ? AF_RGBA : AF_RGB);
		img->data = new unsigned char[img->w * img->h * img->bpp];
		unsigned char* p = img->data;
		unsigned char* src = (unsigned char*)rect.pBits;
		int x;
		memset(p, 255, img->w * img->h * img->bpp);
		for_iter (y, 0, img->h)
		{
			for (x = 0; x < img->w; x++, p += bpp)
			{
				p[0] = src[x * bpp + 2];
				p[1] = src[x * bpp + 1];
				p[2] = src[x * bpp];
			}
			src += rect.Pitch;
		}
		buffer->UnlockRect();
		buffer->Release();
		return img;
		*/
		return NULL;
	}
	
	void DirectX11_RenderSystem::presentFrame()
	{
		this->swapChain->Present(1, 0);
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), nullptr);
	}

}

#endif
