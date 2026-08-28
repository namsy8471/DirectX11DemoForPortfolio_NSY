#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#include <dinput.h>
#include <wrl/client.h>

#include "Engine/Input/InputSnapshot.h"

class InputClass
{
public:
	InputClass() = default;
	InputClass(const InputClass&) = delete;
	InputClass& operator=(const InputClass&) = delete;
	InputClass(InputClass&&) = delete;
	InputClass& operator=(InputClass&&) = delete;
	~InputClass();

	bool Initialize(HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown() noexcept;
	bool Frame(float frameTime);

	[[nodiscard]] const Engine::Input::InputSnapshot& GetSnapshot() const noexcept;

	// Compatibility accessors for call sites that have not moved to InputSnapshot yet.
	void GetMouseLocation(int& mouseX, int& mouseY) const noexcept;
	[[nodiscard]] bool IsEscapePressed() const noexcept;
	[[nodiscard]] bool IsLeftMouseButtonDown() const noexcept;

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput() noexcept;

private:
	Microsoft::WRL::ComPtr<IDirectInput8> m_directInput;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> m_keyboard;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> m_mouse;

	unsigned char m_keyboardState[256] = {};
	DIMOUSESTATE m_mouseState = {};

	int m_screenWidth = 0;
	int m_screenHeight = 0;
	int m_mouseX = 0;
	int m_mouseY = 0;
	Engine::Input::InputSnapshot m_snapshot;
};

#endif
