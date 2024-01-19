////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "stdafx.h"
#include "d3dclass.h"
#include "cameraclass.h"

// Input
#include "inputclass.h"

// Sound
#include "SoundClass.h"

// Model(3D)
#include "modelclass.h"
#include "textureshaderclass.h"

// Bitmap(2D)
#include "bitmapclass.h"

// Text
#include "textclass.h"
#include "fontshaderclass.h"

// Light class
#include "LightClass.h"
#include "lightshaderclass.h"

// Shadow class
#include "ShadowShaderClass.h"
#include "RenderTextureClass.h"
#include "DepthShaderClass.h"
#include "OrthoWindowClass.h"
#include "SoftShadowShaderClass.h"
#include "HorizontalBlurShaderClass.h"
#include "VerticalBlurShaderClass.h"

// Terrain & Terrain Shader
#include "TerrainClass.h"
#include "TerrainShaderClass.h"

// Skydorm
#include "SkyDomeClass.h"
#include "SkyDomeShaderClass.h"

// FireShader
#include "ModelClassForNoiseFilter.h"
#include "FireShaderClass.h"

// Foliage
#include "foliageclass.h"
#include "foliageshaderclass.h"


#include <vector>

#define MAX_OBJS 4
/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = false;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

const int SHADOWMAP_WIDTH = 1024;
const int SHADOWMAP_HEIGHT = 1024;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND, HINSTANCE);
	void Shutdown();
	bool Frame(int, int, float);


	CameraClass* GetCam();
private:

	enum class StageEnum {
		Title = 0,
		Stage
	};

	bool HandleInput(float);
	void TestIntersection();
	bool RaySphereIntersect(XMFLOAT3, XMFLOAT3, float);

	bool RenderSceneToTexture();
	bool RenderBlackAndWhiteShadows();
	bool DownSampleTexture();
	bool RenderHorizontalBlurToTexture();
	bool RenderVerticalBlurToTexture();
	bool UpSampleTexture();
	bool Render(float);

private:
	
	StageEnum currentStage = StageEnum::Title;

	unsigned int wholeObj;
	unsigned int wholePoly;

	// For input
	InputClass* m_Input = nullptr;
	bool m_beginCheck = false;
	int m_screenWidth = 0;
	int m_screenHeight = 0;

	// For Sound
	SoundClass* m_Sound = nullptr;

	D3DClass* m_D3D;
	CameraClass* m_Camera;

	// For model and Bitmaps
	vector<ModelClass*> m_Models;
	BitmapClass* m_MouseCursor = nullptr;
	vector<BitmapClass*> m_Crosshairs;


	TextureShaderClass* m_TextureShader;

	const WCHAR* obj_Names[MAX_OBJS];
	const WCHAR* tex_Names[MAX_OBJS];
	const WCHAR* bit_Names[2];

	// For Text
	TextClass* m_Text;

	// For Fire Effect
	ModelClassForNoiseFilter* m_ModelForFire = nullptr;
	FireShaderClass* m_FireShader = nullptr;

	// For Terrain
	TerrainClass* m_Terrain;
	TerrainShaderClass* m_TerrainShader;

	// For Light
	LightShaderClass* m_LightShader;
	LightClass* m_Light;

	// For Render Texture
	RenderTextureClass* m_RenderTexture;
	RenderTextureClass* m_BlackWhiteRenderTexture = nullptr;
	RenderTextureClass* m_DownSampleTexure = nullptr;
	RenderTextureClass* m_HorizontalBlurTexture = nullptr;
	RenderTextureClass* m_VerticalBlurTexture = nullptr;
	RenderTextureClass* m_UpSampleTexure = nullptr;

	// For shadow
	DepthShaderClass* m_DepthShader;
	ShadowShaderClass* m_ShadowShader;
	HorizontalBlurShaderClass* m_HorizontalBlurShader = nullptr;
	VerticalBlurShaderClass* m_VerticalBlurShader = nullptr;
	SoftShadowShaderClass* m_SoftShadowShader = nullptr;

	OrthoWindowClass* m_SmallWindow = nullptr;
	OrthoWindowClass* m_FullScreenWindow = nullptr;
	
	// For SkyDome
	SkyDomeClass* m_SkyDome = nullptr;
	SkyDomeShaderClass* m_SkyDomeShader = nullptr;

	// For foliage
	FoliageClass* m_Foliage = nullptr;
	FoliageShaderClass* m_FoliageShader = nullptr;
};

#endif