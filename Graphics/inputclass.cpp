#include "inputclass.h"

#include <algorithm>
#include <cstring>

InputClass::~InputClass()
{
	Shutdown();
}

bool InputClass::Initialize(HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight)
{
	Shutdown();

	m_screenWidth = (std::max)(screenWidth, 0);
	m_screenHeight = (std::max)(screenHeight, 0);
	m_mouseX = m_screenWidth / 2;
	m_mouseY = m_screenHeight / 2;
	m_snapshot.cursorX = m_mouseX;
	m_snapshot.cursorY = m_mouseY;

	HRESULT result = DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		reinterpret_cast<void**>(m_directInput.ReleaseAndGetAddressOf()),
		nullptr);
	if (FAILED(result))
	{
		Shutdown();
		return false;
	}

	result = m_directInput->CreateDevice(GUID_SysKeyboard, m_keyboard.ReleaseAndGetAddressOf(), nullptr);
	if (FAILED(result))
	{
		Shutdown();
		return false;
	}

	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		Shutdown();
		return false;
	}

	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		Shutdown();
		return false;
	}

	result = m_keyboard->Acquire();
	if (FAILED(result) &&
		result != DIERR_OTHERAPPHASPRIO &&
		result != DIERR_INPUTLOST &&
		result != DIERR_NOTACQUIRED)
	{
		Shutdown();
		return false;
	}

	result = m_directInput->CreateDevice(GUID_SysMouse, m_mouse.ReleaseAndGetAddressOf(), nullptr);
	if (FAILED(result))
	{
		Shutdown();
		return false;
	}

	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result))
	{
		Shutdown();
		return false;
	}

	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result))
	{
		Shutdown();
		return false;
	}

	result = m_mouse->Acquire();
	if (FAILED(result) &&
		result != DIERR_OTHERAPPHASPRIO &&
		result != DIERR_INPUTLOST &&
		result != DIERR_NOTACQUIRED)
	{
		Shutdown();
		return false;
	}

	return true;
}

void InputClass::Shutdown() noexcept
{
	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse.Reset();
	}

	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard.Reset();
	}

	m_directInput.Reset();
	std::memset(m_keyboardState, 0, sizeof(m_keyboardState));
	m_mouseState = {};
	m_snapshot = {};
	m_screenWidth = 0;
	m_screenHeight = 0;
	m_mouseX = 0;
	m_mouseY = 0;
}

bool InputClass::Frame(float frameTime)
{
	(void)frameTime;

	if (!m_keyboard || !m_mouse)
	{
		return false;
	}

	if (!ReadKeyboard() || !ReadMouse())
	{
		return false;
	}

	ProcessInput();
	return true;
}

bool InputClass::ReadKeyboard()
{
	HRESULT result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), m_keyboardState);
	if (SUCCEEDED(result))
	{
		return true;
	}

	std::memset(m_keyboardState, 0, sizeof(m_keyboardState));
	if (result != DIERR_INPUTLOST && result != DIERR_NOTACQUIRED)
	{
		return false;
	}

	result = m_keyboard->Acquire();
	if (FAILED(result))
	{
		return result == DIERR_OTHERAPPHASPRIO ||
			result == DIERR_INPUTLOST ||
			result == DIERR_NOTACQUIRED;
	}

	result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), m_keyboardState);
	if (FAILED(result))
	{
		std::memset(m_keyboardState, 0, sizeof(m_keyboardState));
		return result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED;
	}

	return true;
}

bool InputClass::ReadMouse()
{
	HRESULT result = m_mouse->GetDeviceState(sizeof(m_mouseState), &m_mouseState);
	if (SUCCEEDED(result))
	{
		return true;
	}

	m_mouseState = {};
	if (result != DIERR_INPUTLOST && result != DIERR_NOTACQUIRED)
	{
		return false;
	}

	result = m_mouse->Acquire();
	if (FAILED(result))
	{
		return result == DIERR_OTHERAPPHASPRIO ||
			result == DIERR_INPUTLOST ||
			result == DIERR_NOTACQUIRED;
	}

	result = m_mouse->GetDeviceState(sizeof(m_mouseState), &m_mouseState);
	if (FAILED(result))
	{
		m_mouseState = {};
		return result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED;
	}

	return true;
}

void InputClass::ProcessInput() noexcept
{
	m_mouseX = std::clamp(m_mouseX + static_cast<int>(m_mouseState.lX), 0, m_screenWidth);
	m_mouseY = std::clamp(m_mouseY + static_cast<int>(m_mouseState.lY), 0, m_screenHeight);

	m_snapshot.escapePressed = (m_keyboardState[DIK_ESCAPE] & 0x80) != 0;
	m_snapshot.primaryActionDown = (m_mouseState.rgbButtons[0] & 0x80) != 0;
	m_snapshot.cursorX = m_mouseX;
	m_snapshot.cursorY = m_mouseY;
	m_snapshot.moveRight = static_cast<float>(
		((m_keyboardState[DIK_D] & 0x80) != 0) - ((m_keyboardState[DIK_A] & 0x80) != 0));
	m_snapshot.moveForward = static_cast<float>(
		((m_keyboardState[DIK_W] & 0x80) != 0) - ((m_keyboardState[DIK_S] & 0x80) != 0));
	m_snapshot.lookX = static_cast<float>(m_mouseState.lX);
	m_snapshot.lookY = static_cast<float>(m_mouseState.lY);
}

const Engine::Input::InputSnapshot& InputClass::GetSnapshot() const noexcept
{
	return m_snapshot;
}

void InputClass::GetMouseLocation(int& mouseX, int& mouseY) const noexcept
{
	mouseX = m_snapshot.cursorX;
	mouseY = m_snapshot.cursorY;
}

bool InputClass::IsEscapePressed() const noexcept
{
	return m_snapshot.escapePressed;
}

bool InputClass::IsLeftMouseButtonDown() const noexcept
{
	return m_snapshot.primaryActionDown;
}
