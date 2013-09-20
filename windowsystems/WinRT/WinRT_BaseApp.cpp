﻿/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "Keys.h"
#include "Platform.h"
#include "SystemDelegate.h"
#include "Window.h"
#include "WinRT.h"
#include "WinRT_BaseApp.h"

using namespace Windows::Foundation;
using namespace Windows::UI::Core;

namespace april
{
	WinRT_BaseApp::WinRT_BaseApp()
	{
		//april::WinRT::App = this;
		this->scrollHorizontal = false;
		this->mouseMoveMessagesCount = 0;
		this->currentButton = april::AK_NONE;
	}

	WinRT_BaseApp::~WinRT_BaseApp()
	{
	}

	void WinRT_BaseApp::assignEvents(CoreWindow^ window)
	{
		window->SizeChanged +=
			ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
				this, &WinRT_BaseApp::OnWindowSizeChanged);
		window->VisibilityChanged +=
			ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(
				this, &WinRT_BaseApp::OnVisibilityChanged);
		window->PointerPressed +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_BaseApp::OnTouchDown);
		window->PointerReleased +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_BaseApp::OnTouchUp);
		window->PointerMoved +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_BaseApp::OnTouchMove);
		window->PointerWheelChanged +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_BaseApp::OnMouseScroll);
		window->KeyDown +=
			ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
				this, &WinRT_BaseApp::OnKeyDown);
		window->KeyUp +=
			ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
				this, &WinRT_BaseApp::OnKeyUp);
		window->CharacterReceived +=
			ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(
				this, &WinRT_BaseApp::OnCharacterReceived);
		window->Closed +=
			ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(
				this, &WinRT_BaseApp::OnWindowClosed);
	}

	void WinRT_BaseApp::OnSuspend(_In_ Object^ sender, _In_ SuspendingEventArgs^ args)
	{
		if (april::window != NULL && WinRT::Interface->canSuspendResume())
		{
			april::window->handleFocusChangeEvent(false);
		}
	}

	void WinRT_BaseApp::OnResume(_In_ Object^ sender, _In_ Object^ args)
	{
		if (april::window != NULL && WinRT::Interface->canSuspendResume())
		{
			april::window->handleFocusChangeEvent(true);
		}
	}

	void WinRT_BaseApp::OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleQuitRequest(false);
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
		WinRT::Interface->updateViewState();
		if (april::window != NULL)
		{
			april::SystemDelegate* systemDelegate = april::window->getSystemDelegate();
			if (systemDelegate != NULL)
			{
				systemDelegate->onWindowSizeChanged((int)args->Size.Width, (int)args->Size.Height, true);
			}
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
	{
		WinRT::Interface->updateViewState();
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL)
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		this->currentButton = april::AK_LBUTTON;
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			if (args->CurrentPoint->Properties->IsRightButtonPressed)
			{
				this->currentButton = april::AK_RBUTTON;
			}
			else if (args->CurrentPoint->Properties->IsMiddleButtonPressed)
			{
				this->currentButton = april::AK_MBUTTON;
			}
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_DOWN, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
#endif
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
				this->pointerIds += id;
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_DOWN, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL)
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_UP, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
#endif
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			else
			{
				this->pointerIds.remove_at(index);
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_UP, position, index);
#ifndef _WINP8
			break;
		}
		this->currentButton = april::AK_NONE;
#endif
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL)
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			this->mouseMoveMessagesCount++;
			if (this->mouseMoveMessagesCount >= 10)
			{
				april::window->setInputMode(april::Window::MOUSE);
			}
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_MOVE, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
#endif
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_MOVE, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL)
		{
			return;
		}
		april::window->setInputMode(april::Window::MOUSE);
		float _wheelDelta = (float)args->CurrentPoint->Properties->MouseWheelDelta / WHEEL_DELTA;
		if (this->scrollHorizontal ^ args->CurrentPoint->Properties->IsHorizontalMouseWheel)
		{
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_SCROLL,
				gvec2(-(float)_wheelDelta, 0.0f), april::AK_NONE);
		}
		else
		{
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_SCROLL,
				gvec2(0.0f, -(float)_wheelDelta), april::AK_NONE);
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		if (april::window == NULL)
		{
			return;
		}
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = true;
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		if (april::window == NULL)
		{
			return;
		}
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::AKEYEVT_UP, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = false;
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		if (april::window == NULL)
		{
			return;
		}
		april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, AK_NONE, args->KeyCode);
		args->Handled = true;
	}
	
	gvec2 WinRT_BaseApp::_translatePosition(float x, float y)
	{
		static int w = 0;
		static int h = 0;
		if (w == 0 || h == 0)
		{
			gvec2 resolution = april::getSystemInfo().displayResolution;
			w = hround(resolution.x);
			h = hround(resolution.y);
			CHECK_SWAP(w, h);
		}
#ifdef _WINP8
		int rotation = WinRT::getScreenRotation();
		if (rotation == 90)
		{
			x = w - x;
			hswap(x, y);
		}
		else if (rotation == 180)
		{
			y = h - y;
			x = w - x;
		}
		else if (rotation == 270)
		{
			y = h - y;
			hswap(x, y);
		}
#endif
		int width = april::window->getWidth();
		int height = april::window->getHeight();
		CHECK_SWAP(width, height);
		if (w == width && h == height)
		{
			return gvec2(x, y);
		}
		return gvec2((float)(int)(x * width / w), (float)(int)(y * height / h));
	}

}
#endif
