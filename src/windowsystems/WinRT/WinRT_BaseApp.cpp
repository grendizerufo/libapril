﻿/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINRT_WINDOW
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "DirectX11_RenderSystem.h"
#include "Keys.h"
#include "Platform.h"
#include "Rendersystem.h"
#include "SystemDelegate.h"
#include "Window.h"
#include "WinRT.h"
#include "WinRT_BaseApp.h"
#include "WinRT_Window.h"

using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;

namespace april
{
	WinRT_BaseApp::WinRT_BaseApp()
	{
		DisplayInformation::AutoRotationPreferences = (DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped);
		this->scrollHorizontal = false;
		this->mouseMoveMessagesCount = 0;
		this->startTime = get_system_tick_count();
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
		DisplayInformation::GetForCurrentView()->OrientationChanged +=
			ref new Windows::Foundation::TypedEventHandler<DisplayInformation^, Object^>(
				this, &WinRT_BaseApp::OnOrientationChanged);
		InputPane::GetForCurrentView()->Showing +=
			ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(
				this, &WinRT_BaseApp::OnVirtualKeyboardShow);
		InputPane::GetForCurrentView()->Hiding +=
			ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(
				this, &WinRT_BaseApp::OnVirtualKeyboardHide);
	}

	void WinRT_BaseApp::OnSuspend(_In_ Object^ sender, _In_ SuspendingEventArgs^ args)
	{
		DX11_RENDERSYS->trim(); // required since Win 8.1
		this->handleFocusChange(false);
	}

	void WinRT_BaseApp::OnResume(_In_ Object^ sender, _In_ Object^ args)
	{
		this->handleFocusChange(true);
	}

	void WinRT_BaseApp::OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleQuitRequest(false);
		}
		args->Handled = true;
	}

	void WinRT_BaseApp::handleFocusChange(bool focused)
	{
		this->resetTouches();
		if (april::window != NULL && april::window->isFocused() != focused)
		{
			april::window->handleFocusChangeEvent(focused);
		}
	}

	void WinRT_BaseApp::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
		this->resetTouches();
		april::SystemInfo info = april::getSystemInfo(); // outside, because the displayResolution needs to be updated every time
		if (april::window != NULL)
		{
			int width = (int)(args->Size.Width * info.displayDpi / 96.0f);
			int height = (int)(args->Size.Height * info.displayDpi / 96.0f);
			((WinRT_Window*)april::window)->changeSize(width, height);
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
	{
		this->resetTouches();
		args->Handled = true;
	}

	void WinRT_BaseApp::OnOrientationChanged(_In_ DisplayInformation^ sender, _In_ Object^ args)
	{
		this->resetTouches();
	}
	
	void WinRT_BaseApp::OnVirtualKeyboardShow(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleVirtualKeyboardChangeEvent(true, args->OccludedRect.Height / CoreWindow::GetForCurrentThread()->Bounds.Height);
		}
		this->resetTouches();
	}

	void WinRT_BaseApp::OnVirtualKeyboardHide(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleVirtualKeyboardChangeEvent(false, 0.0f);
		}
		this->resetTouches();
	}

	void WinRT_BaseApp::OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
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
			april::window->queueMouseEvent(april::Window::MOUSE_DOWN, position, this->currentButton);
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
			april::window->queueTouchEvent(april::Window::MOUSE_DOWN, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::MOUSE_UP, position, this->currentButton);
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
			april::window->queueTouchEvent(april::Window::MOUSE_UP, position, index);
#ifndef _WINP8
			break;
		}
		this->currentButton = april::AK_NONE;
#endif
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			this->mouseMoveMessagesCount++;
			if (this->mouseMoveMessagesCount >= 10)
			{
				april::window->setInputMode(april::Window::MOUSE);
			}
			april::window->queueMouseEvent(april::Window::MOUSE_MOVE, position, this->currentButton);
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
			april::window->queueTouchEvent(april::Window::MOUSE_MOVE, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::window->setInputMode(april::Window::MOUSE);
		float _wheelDelta = (float)args->CurrentPoint->Properties->MouseWheelDelta / WHEEL_DELTA;
		if (this->scrollHorizontal ^ args->CurrentPoint->Properties->IsHorizontalMouseWheel)
		{
			april::window->queueMouseEvent(april::Window::MOUSE_SCROLL,
				gvec2(-(float)_wheelDelta, 0.0f), april::AK_NONE);
		}
		else
		{
			april::window->queueMouseEvent(april::Window::MOUSE_SCROLL,
				gvec2(0.0f, -(float)_wheelDelta), april::AK_NONE);
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::KEY_DOWN, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = true;
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::KEY_UP, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = false;
		}
		if (key == AK_RETURN)
		{
			april::window->terminateKeyboardHandling();
		}
		args->Handled = true;
	}
	
	void WinRT_BaseApp::OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::window->queueKeyEvent(april::Window::KEY_DOWN, AK_NONE, args->KeyCode);
		args->Handled = true;
	}

	void WinRT_BaseApp::resetTouches()
	{
		for_iter_r (i, this->pointerIds.size(), 0)
		{
			april::window->queueTouchEvent(april::Window::MOUSE_CANCEL, gvec2(), i);
		}
		this->pointerIds.clear();
	}
	
	gvec2 WinRT_BaseApp::_transformPosition(float x, float y)
	{
		april::SystemInfo info = april::getSystemInfo();
		// WinRT is dumb
		x *= info.displayDpi / 96.0f;
		y *= info.displayDpi / 96.0f;
		int w = hround(info.displayResolution.x);
		int h = hround(info.displayResolution.y);
		int width = april::window->getWidth();
		int height = april::window->getHeight();
		if (w == width && h == height)
		{
			return gvec2(x, y);
		}
		return gvec2((float)(int)(x * width / w), (float)(int)(y * height / h));
	}

}
#endif
