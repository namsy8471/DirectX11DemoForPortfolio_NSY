////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

#include <dinput.h>
////////////////////////////////////////////////////////////////////////////////
// Class name: InputClass
////////////////////////////////////////////////////////////////////////////////

//class GraphicsClass;
class CameraClass;

class InputClass
{
public:
	InputClass();
	InputClass(const InputClass&);
	~InputClass();

	bool Initialize(HINSTANCE, HWND, int, int);

	void Shutdown();
	bool Frame(float);

	void GetMouseLocation(int&, int&);
	bool IsEscapePressed();
	bool IsLeftMouseButtonDown();

	void SetCamera(CameraClass*);

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput(float);

private:

	CameraClass* m_camera;

	LPDIRECTINPUT8 m_directInput;

	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;

	unsigned char m_keyboardState[256] = { 0, };
	DIMOUSESTATE m_mouseState;
	DIMOUSESTATE m_mouseLastState;

	int m_screenWidth = 0;
	int m_screenHeight = 0;
	int m_mouseX = 0;
	int m_mouseY = 0;
};

#endif