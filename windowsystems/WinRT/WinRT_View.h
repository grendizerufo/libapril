/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinRT View.

#ifdef _WIN32
#ifndef APRIL_WINRT_VIEW_H
#define APRIL_WINRT_VIEW_H
#include <hltypes/hplatform.h>
#if _HL_WINRT
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include <windows.h>
#include <agile.h>

using namespace Microsoft::WRL;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;

namespace april
{
	class WinRT
	{
	public:
		~WinRT() { }

		static void (*Init)(const harray<hstr>&);
		static void (*Destroy)();
		static harray<hstr> Args;

	private:
		WinRT() { }

	};

	ref class WinRT_View : public IFrameworkView
	{
	public:
		virtual void Initialize(_In_ CoreApplicationView^ applicationView);
		virtual void Uninitialize();
		virtual void SetWindow(_In_ CoreWindow^ window);
		virtual void Load(_In_ Platform::String^ entryPoint);
		virtual void Run();

		void* getBackendId();

		void OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args);
		void OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args);

		void checkEvents();

	private: // has to be private
		Platform::Agile<CoreWindow> window;


		/*
		ComPtr<IDXGISwapChain1> swapChain;
		ComPtr<ID3D11Device1> d3dDevice;
		ComPtr<ID3D11DeviceContext1> d3dDeviceContext;
		ComPtr<ID3D11RenderTargetView> renderTargetView;
		*/

	};

}

#endif
#endif
#endif