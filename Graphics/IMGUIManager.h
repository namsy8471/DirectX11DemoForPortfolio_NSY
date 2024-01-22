#pragma once
#include <Windows.h>
#include <d3d11.h>

class IMGUIManager
{
private:
	static IMGUIManager* _instance;

public:
	IMGUIManager();
	~IMGUIManager();
	static IMGUIManager* Instance();
	bool Initialize(HWND, ID3D11Device*, ID3D11DeviceContext*);
	void Frame();
	void Render();
	void ShutDown();
};

