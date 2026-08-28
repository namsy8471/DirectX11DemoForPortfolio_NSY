#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

/////////////
// LINKING //
/////////////
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3dcompiler.lib")



//////////////
// INCLUDES //
//////////////
#include <d3d11_1.h>
#include <dinput.h>
#include <mmsystem.h>
#include <dsound.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "Engine/Rendering/Textures/DDSTextureLoader.h"	// DDS ���� ó��
using namespace DirectX;

///////////////////////////
//  warning C4316 ó����  //
///////////////////////////
#include "Engine/Core/AlignedAllocationPolicy.h"
