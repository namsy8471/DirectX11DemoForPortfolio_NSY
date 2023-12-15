////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////

#include "graphicsclass.h"

GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;

	m_Models.clear();

	m_TextureShader = 0;
	m_Crosshairs.clear();

	m_Text = 0;

	m_Terrain = 0;
	m_TerrainShader = 0;

	m_Light = 0;
	m_LightShader = 0;

	m_RenderTexture = 0;
	m_DepthShader = 0;
	m_ShadowShader = 0;

	wholeObj = 0;
	wholePoly = 0;

	obj_Names[0] = { L"./data/bear.obj" };
	obj_Names[1] = { L"./data/11560_wild_turkey_male_v2_l2.obj"};
	obj_Names[2] = { L"./data/11561_Turkey - Wild_female_v4_l1.obj" };
	obj_Names[3] = { L"./data/horse.obj" };

	tex_Names[0] = { L"./data/bear.dds" };
	tex_Names[1] = { L"./data/11560_wild_turkey_male_v2_l2.dds" };
	tex_Names[2] = { L"./data/11561_Turkey - Wild_female_v4_l1.dds" };
	tex_Names[3] = { L"./data/horse.dds" };

	m_SkyDome = 0;
	m_ShadowShader = 0;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd, HINSTANCE hInstance)
{
	bool result;

	XMMATRIX baseViewMatrix;

	// 화면 너비와 높이를 저장합니다.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// 카메라 포지션을 설정한다
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->RenderBaseViewMatrix();


	result = m_Camera->Initialize();
	if (!result)
	{
		return false;
	}
	
	// 입력 개체를 만듭니다.
	m_Input = new InputClass;
	if (!m_Input)
	{
		return false;
	}

	// 입력 개체를 초기화합니다.
	result = m_Input->Initialize(hInstance, hwnd, screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	m_Input->SetCamera(m_Camera);

	// Create the model object.
	for (int i = 0; i < MAX_OBJS; i++)
	{
		m_Models.push_back(new ModelClass);
		if (!m_Models[i])
		{
			MessageBox(hwnd, L"m_Models[i] is not exist.", L"Error", MB_OK);
			return false;
		}

		result = m_Models[i]->Initialize(hwnd, m_D3D->GetDevice(), obj_Names[i], tex_Names[i]);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}

		wholeObj++;
		wholePoly += m_Models[i]->CountPolygons();
	}

	// 모델 위치 초기화
	m_Models[0]->SetPosition(3.f, -2.5f, -1.f);

	m_Models[1]->SetPosition(8.f, -2.5f, -1.f);
	m_Models[1]->SetRotationByDegrees(90.f, 90.f, 0.f);
	m_Models[1]->SetScale(0.01f, 0.01f, 0.01f);

	m_Models[2]->SetPosition(0.f, -2.5f, -1.f);
	m_Models[2]->SetRotationByDegrees(90.f, 90.f, 0.f);
	m_Models[2]->SetScale(0.01f, 0.01f, 0.01f);

	m_Models[3]->SetPosition(5.f, -2.5f, -1.f);
	m_Models[3]->SetRotationByDegrees(90.f, 0.f, 0.f);
	m_Models[3]->SetScale(0.002f, 0.002f, 0.002f);


	// 불 효과 모델 객체 생성
	m_ModelForFire = new ModelClassForNoiseFilter;
	if (!m_ModelForFire)
	{
		return false;
	}

	wholeObj++;
	wholePoly += m_ModelForFire->GetIndexCount() / 3;

	// 모델 객체 초기화
	if (!m_ModelForFire->Initialize(hwnd, m_D3D->GetDevice(), "./data/square.txt", L"./data/fire01.dds",
		L"./data/noise01.dds", L"./data/alpha01.dds"))
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// 파이어 쉐이더 객체를 생성한다.
	m_FireShader = new FireShaderClass;
	if (!m_FireShader)
	{
		return false;
	}

	// 화재 쉐이더 객체를 초기화합니다.
	if (!m_FireShader->Initialize(m_D3D->GetDevice(), hwnd))
	{
		MessageBox(hwnd, L"Could not initialize the fire shader object.", L"Error", MB_OK);
		return false;
	}

	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	for (int i = 0; i < 4; i++)
	{
		m_Crosshairs.push_back(new BitmapClass);
		if (!m_Crosshairs[i])
		{
			return false;
		}

		result = m_Crosshairs[i]->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, L"./data/Crosshair.dds", 256, 256);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the Crosshair bitmap object.", L"Error", MB_OK);
			return false;
		}

		wholeObj++;
	}

	m_Crosshairs[0]->SetPosition(-5, 0.1f, 0);
	m_Crosshairs[1]->SetPosition(5, 0.1f, 0);
	m_Crosshairs[2]->SetPosition(0.1f, 5, 0);
	m_Crosshairs[3]->SetPosition(0.1f, -5, 0);

	m_Crosshairs[0]->SetScale(0.01f, 0.005f, 1);
	m_Crosshairs[1]->SetScale(0.01f, 0.005f, 1);
	m_Crosshairs[2]->SetScale(0.008f, 0.01f, 1);
	m_Crosshairs[3]->SetScale(0.008f, 0.01f, 1);

	m_MouseCursor = new BitmapClass;
	if (!m_MouseCursor)
	{
		return false;
	}

	wholeObj++;

	result = m_MouseCursor->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, L"./data/mouse.dds", 32, 32);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}

	m_Camera->Render();
	m_Camera->GetViewMatrix(baseViewMatrix);

	// Create the text object.
	m_Text = new TextClass;
	if (!m_Text)
	{
		return false;
	}

	// Initialize the text object.
	result = m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}


	// 지형 객체를 생성합니다.
	m_Terrain = new TerrainClass;
	if (!m_Terrain)
	{
		return false;
	}

	// 지형 객체를 초기화 합니다.
	result = m_Terrain->Initialize(m_D3D->GetDevice(), "./data/heightmap01.bmp", L"./data/dirt01.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
		return false;
	}

	// 지형 쉐이더 객체를 생성합니다.
	m_TerrainShader = new TerrainShaderClass;
	if (!m_TerrainShader)
	{
		return false;
	}

	// 지형 쉐이더 객체를 초기화 합니다.
	result = m_TerrainShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the terrain shader object.", L"Error", MB_OK);
		return false;
	}

	// 조명 객체를 생성합니다.
	m_Light = new LightClass;
	if (!m_Light)
	{
		return false;
	}

	// 조명 객체를 초기화 합니다.
	//m_Light->SetAmbientColor(XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f));
	m_Light->SetAmbientColor(XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f));
	m_Light->SetDiffuseColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_Light->SetLookAt(XMFLOAT3(0.0f, 0.0f, 10.0f));
	m_Light->SetSpecularColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_Light->SetSpecularPower(1000.f);
	m_Light->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);


		// 렌더링을 텍스처 오브젝트에 생성한다.
	m_RenderTexture = new RenderTextureClass;
	if (!m_RenderTexture)
	{
		return false;
	}

	// 렌더링을 텍스처 오브젝트에 초기화한다.
	result = m_RenderTexture->Initialize(m_D3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
		return false;
	}

	// 깊이 셰이더 개체를 만듭니다.
	m_DepthShader = new DepthShaderClass;
	if (!m_DepthShader)
	{
		return false;
	}

	// 깊이 셰이더 개체를 초기화합니다.
	result = m_DepthShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the depth shader object.", L"Error", MB_OK);
		return false;
	}

	// 흑백 렌더링을 텍스처 오브젝트에 생성한다.
	m_BlackWhiteRenderTexture = new RenderTextureClass;
	if (!m_BlackWhiteRenderTexture)
	{
		return false;
	}

	// 흑백 렌더링을 텍스처 오브젝트에 초기화한다.
	result = m_BlackWhiteRenderTexture->Initialize(m_D3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the black and white render to texture object.", L"Error", MB_OK);
		return false;
	}

	// 그림자 셰이더 개체를 만듭니다.
	m_ShadowShader = new ShadowShaderClass;
	if (!m_ShadowShader)
	{
		return false;
	}

	// 그림자 쉐이더 객체를 초기화합니다.
	result = m_ShadowShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the shadow shader object.", L"Error", MB_OK);
		return false;
	}

	// 크기를 샘플로 설정합니다.
	int downSampleWidth = SHADOWMAP_WIDTH / 2;
	int downSampleHeight = SHADOWMAP_HEIGHT / 2;

	// 다운 샘플 렌더링을 텍스처 오브젝트에 생성한다.
	m_DownSampleTexure = new RenderTextureClass;
	if (!m_DownSampleTexure)
	{
		return false;
	}

	// 다운 샘플 렌더를 텍스처 오브젝트로 초기화한다.
	result = m_DownSampleTexure->Initialize(m_D3D->GetDevice(), downSampleWidth, downSampleHeight, 100.0f, 1.0f);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the down sample render to texture object.", L"Error", MB_OK);
		return false;
	}

	// 작은 ortho 윈도우 객체를 만듭니다.
	m_SmallWindow = new OrthoWindowClass;
	if (!m_SmallWindow)
	{
		return false;
	}

	// 작은 ortho 윈도우 객체를 초기화합니다.
	result = m_SmallWindow->Initialize(m_D3D->GetDevice(), downSampleWidth, downSampleHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the small ortho window object.", L"Error", MB_OK);
		return false;
	}

	// 텍스처 객체에 수평 블러 렌더링을 만듭니다.
	m_HorizontalBlurTexture = new RenderTextureClass;
	if (!m_HorizontalBlurTexture)
	{
		return false;
	}

	// 텍스처 객체에 수평 블러 렌더링을 초기화합니다.
	result = m_HorizontalBlurTexture->Initialize(m_D3D->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the horizontal blur render to texture object.", L"Error", MB_OK);
		return false;
	}

	// 수평 블러 쉐이더 객체를 만듭니다.
	m_HorizontalBlurShader = new HorizontalBlurShaderClass;
	if (!m_HorizontalBlurShader)
	{
		return false;
	}

	// 수평 블러 쉐이더 객체를 초기화합니다.
	result = m_HorizontalBlurShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the horizontal blur shader object.", L"Error", MB_OK);
		return false;
	}

	// 텍스처 오브젝트에 수직 블러 렌더를 만듭니다.
	m_VerticalBlurTexture = new RenderTextureClass;
	if (!m_VerticalBlurTexture)
	{
		return false;
	}

	// 텍스처 오브젝트에 수직 블러 렌더를 초기화합니다.
	result = m_VerticalBlurTexture->Initialize(m_D3D->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the vertical blur render to texture object.", L"Error", MB_OK);
		return false;
	}

	// 수직 블러 셰이더 오브젝트를 생성한다.
	m_VerticalBlurShader = new VerticalBlurShaderClass;
	if (!m_VerticalBlurShader)
	{
		return false;
	}

	// 수직 블러 쉐이더 객체를 초기화합니다.
	result = m_VerticalBlurShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the vertical blur shader object.", L"Error", MB_OK);
		return false;
	}

	// 텍스쳐 객체에 업 샘플 렌더를 생성한다.
	m_UpSampleTexure = new RenderTextureClass;
	if (!m_UpSampleTexure)
	{
		return false;
	}

	// up 샘플 렌더를 텍스처 오브젝트로 초기화한다.
	result = m_UpSampleTexure->Initialize(m_D3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, 0.1f);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the up sample render to texture object.", L"Error", MB_OK);
		return false;
	}

	// 전체 화면 ortho window 객체를 만듭니다.
	m_FullScreenWindow = new OrthoWindowClass;
	if (!m_FullScreenWindow)
	{
		return false;
	}

	// 전체 화면 ortho window 객체를 초기화합니다.
	result = m_FullScreenWindow->Initialize(m_D3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the full screen ortho window object.", L"Error", MB_OK);
		return false;
	}

	// 부드러운 그림자 쉐이더 객체를 만듭니다.
	m_SoftShadowShader = new SoftShadowShaderClass;
	if (!m_SoftShadowShader)
	{
		return false;
	}

	// 부드러운 그림자 쉐이더 객체를 초기화합니다.
	result = m_SoftShadowShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the soft shadow shader object.", L"Error", MB_OK);
		return false;
	}

	// 스카이 돔 객체를 생성합니다.
	m_SkyDome = new SkyDomeClass;
	if (!m_SkyDome)
	{
		return false;
	}

	// 스카이 돔 객체를 초기화 합니다.
	result = m_SkyDome->Initialize(m_D3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the sky dome object.", L"Error", MB_OK);
		return false;
	}
	else{
		wholeObj++;
		wholePoly += m_SkyDome->GetIndexCount();
	}

	// 스카이 돔 쉐이더 객체를 생성합니다.
	m_SkyDomeShader = new SkyDomeShaderClass;
	if (!m_SkyDomeShader)
	{
		return false;
	}

	// 스카이 돔 쉐이더 객체를 초기화 합니다.
	result = m_SkyDomeShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the sky dome shader object.", L"Error", MB_OK);
		return false;
	}

	// 단풍 객체를 생성합니다.
	m_Foliage = new FoliageClass;
	if (!m_Foliage)
	{
		return false;
	}

	// 단풍 객체를 초기화 합니다.
	result = m_Foliage->Initialize(m_D3D->GetDevice(), L"./data/grass.dds", 2000);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the foliage object.", L"Error", MB_OK);
		return false;
	}


	// 단풍 쉐이더 개체를 만듭니다.
	m_FoliageShader = new FoliageShaderClass;
	if (!m_FoliageShader)
	{
		return false;
	}

	// 단풍 쉐이더 개체를 초기화합니다.
	result = m_FoliageShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the foliage shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the sound object.
	m_Sound = new SoundClass;
	if (!m_Sound)
	{
		return false;
	}

	// Initialize the sound object.
	if (!m_Sound->Initialize(hwnd))
	{
		MessageBox(hwnd, L"Could not initialize Direct Sound.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void GraphicsClass::Shutdown()
{
	// 단풍 쉐이더 객체를 해제합니다.
	if (m_FoliageShader)
	{
		m_FoliageShader->Shutdown();
		delete m_FoliageShader;
		m_FoliageShader = 0;
	}

	// 단풍 객체를 해제합니다.
	if (m_Foliage)
	{
		m_Foliage->Shutdown();
		delete m_Foliage;
		m_Foliage = 0;
	}

	for (auto crosshair : m_Crosshairs)
	{
		if (crosshair)
		{
			crosshair->Shutdown();
			delete crosshair;
			crosshair = 0;
		}
	}

	// fire shader 객체를 해제한다.
	if (m_FireShader)
	{
		m_FireShader->Shutdown();
		delete m_FireShader;
		m_FireShader = 0;
	}

	// 모델 객체를 해제합니다.
	if (m_ModelForFire)
	{
		m_ModelForFire->Shutdown();
		delete m_ModelForFire;
		m_ModelForFire = 0;
	}

	// 비트맵 객체를 해제합니다.
	if (m_MouseCursor)
	{
		m_MouseCursor->Shutdown();
		delete m_MouseCursor;
		m_MouseCursor = 0;
	}

	// Release the sound object.
	if (m_Sound)
	{
		m_Sound->Shutdown();
		delete m_Sound;
		m_Sound = 0;
	}

	// 입력 객체를 해제합니다.
	if (m_Input)
	{
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}
	// 부드러운 그림자 쉐이더 객체를 해제한다.
	if (m_SoftShadowShader)
	{
		m_SoftShadowShader->Shutdown();
		delete m_SoftShadowShader;
		m_SoftShadowShader = 0;
	}

	// 전체 화면 ortho window 객체를 해제합니다.
	if (m_FullScreenWindow)
	{
		m_FullScreenWindow->Shutdown();
		delete m_FullScreenWindow;
		m_FullScreenWindow = 0;
	}

	// up 샘플 렌더를 텍스쳐 객체로 릴리즈한다.
	if (m_UpSampleTexure)
	{
		m_UpSampleTexure->Shutdown();
		delete m_UpSampleTexure;
		m_UpSampleTexure = 0;
	}

	// 수직 블러 쉐이더 객체를 해제한다.
	if (m_VerticalBlurShader)
	{
		m_VerticalBlurShader->Shutdown();
		delete m_VerticalBlurShader;
		m_VerticalBlurShader = 0;
	}

	// 수직 블러 렌더를 텍스처 객체에 놓습니다.
	if (m_VerticalBlurTexture)
	{
		m_VerticalBlurTexture->Shutdown();
		delete m_VerticalBlurTexture;
		m_VerticalBlurTexture = 0;
	}

	// 수평 블러 쉐이더 객체를 해제합니다.
	if (m_HorizontalBlurShader)
	{
		m_HorizontalBlurShader->Shutdown();
		delete m_HorizontalBlurShader;
		m_HorizontalBlurShader = 0;
	}

	// 수평 블러 렌더를 텍스처 객체에 놓습니다.
	if (m_HorizontalBlurTexture)
	{
		m_HorizontalBlurTexture->Shutdown();
		delete m_HorizontalBlurTexture;
		m_HorizontalBlurTexture = 0;
	}

	// 작은 ortho 윈도우 객체를 놓습니다.
	if (m_SmallWindow)
	{
		m_SmallWindow->Shutdown();
		delete m_SmallWindow;
		m_SmallWindow = 0;
	}

	// 다운 샘플 렌더를 텍스쳐 객체로 릴리즈한다.
	if (m_DownSampleTexure)
	{
		m_DownSampleTexure->Shutdown();
		delete m_DownSampleTexure;
		m_DownSampleTexure = 0;
	}

	// 흑백 렌더링을 텍스처로 놓습니다.
	if (m_BlackWhiteRenderTexture)
	{
		m_BlackWhiteRenderTexture->Shutdown();
		delete m_BlackWhiteRenderTexture;
		m_BlackWhiteRenderTexture = 0;
	}

	// 스카이 돔 쉐이더 객체를 해제합니다.
	if (m_SkyDomeShader)
	{
		m_SkyDomeShader->Shutdown();
		delete m_SkyDomeShader;
		m_SkyDomeShader = 0;
	}

	// 스카이 돔 객체를 해제합니다.
	if (m_SkyDome)
	{
		m_SkyDome->Shutdown();
		delete m_SkyDome;
		m_SkyDome = 0;
	}

	// 그림자 쉐이더 객체를 해제합니다.
	if (m_ShadowShader)
	{
		m_ShadowShader->Shutdown();
		delete m_ShadowShader;
		m_ShadowShader = 0;
	}

	// 깊이 셰이더 개체를 해제합니다.
	if (m_DepthShader)
	{
		m_DepthShader->Shutdown();
		delete m_DepthShader;
		m_DepthShader = 0;
	}

	// 렌더 투 텍스쳐 객체를 해제합니다.
	if (m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = 0;
	}

	// 조명 객체를 해제합니다.
	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	// 지형 쉐이더 객체를 해제합니다.
	if (m_TerrainShader)
	{
		m_TerrainShader->Shutdown();
		delete m_TerrainShader;
		m_TerrainShader = 0;
	}

	// 지형 객체를 해제합니다.
	if (m_Terrain)
	{
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	for (int i = 0; i < MAX_OBJS; i++)
	{
		// Release the model objects.
		if (m_Models[i])
		{
			m_Models[i]->Shutdown();
			delete m_Models[i];
			m_Models[i] = 0;
		}

	}

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	// Release the light shader object.
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	return;
}


bool GraphicsClass::Frame(int fps, int cpu, float frameTime)
{
	bool result;

	static float rotation = 0.0f;
	static float lightPositionX = -5.0f;

	
	switch (currentStage)
	{
	case StageEnum::Title:

		if (!m_Text->SetGoalDescription(m_D3D->GetDeviceContext()))
		{
			return false;
		}
		break;

	case StageEnum::Stage:

		m_Sound->PlaySoundForBGM();

		// 초당 프레임 수를 설정합니다.
		if (!m_Text->SetFps(fps, m_D3D->GetDeviceContext()))
		{
			return false;
		}

		// cpu 사용을 설정합니다.
		if (!m_Text->SetCpu(cpu, m_D3D->GetDeviceContext()))
		{
			return false;
		}

		if (!m_Text->SetPolygons(wholePoly, m_D3D->GetDeviceContext()))
		{
			return false;
		}

		if (!m_Text->SetObject(wholeObj, m_D3D->GetDeviceContext()))
		{
			return false;
		}

		if (!m_Text->SetSizeOfScreen(m_D3D->GetDeviceContext()))
		{
			return false;
		}
		break;

	default:
		break;
	}
	


	// 각 프레임의 조명 위치를 업데이트합니다.
	lightPositionX += 0.01f * frameTime;
	if (lightPositionX > 10.0f)
	{
		lightPositionX = -10.0f;
	}

	//빛의 위치를 ​​업데이트합니다.
	m_Light->SetPosition(XMFLOAT3(lightPositionX, 10.0f, -5.0f));
	//m_Light->SetPosition(m_Camera->GetPosition());

	// 카메라 위치를 얻는다.
	XMFLOAT3 cameraPosition = m_Camera->GetPosition();
	
	// 단풍에 대한 프레임 처리를 수행합니다.
	result = m_Foliage->Frame(cameraPosition, m_D3D->GetDeviceContext(), frameTime * 0.01f);
	if (!result)
	{
		return false;
	}

	// 입력 처리를 처리합니다.
	result = HandleInput(frameTime);
	if (!result)
	{
		return false;
	}
	
	// Render the graphics scene.
	result = Render(frameTime);
	if(!result)
	{
		return false;
	}

	return true;
}

bool GraphicsClass::HandleInput(float frameTime)
{
	int mouseX = 0;
	int mouseY = 0;

	// 입력 프레임 처리를 수행합니다.
	if (m_Input->Frame(frameTime) == false)
	{
		return false;
	}

	// 사용자가 이스케이프를 눌렀을 때 응용 프로그램을 종료 할 것인지 확인합니다.
	if (m_Input->IsEscapePressed() == true)
	{
		return false;
	}

	// 마우스 왼쪽 버튼이 눌러 졌는지 확인하십시오
	if (m_Input->IsLeftMouseButtonDown() == true)
	{
		// 마우스로 화면을 클릭 한 다음 교차 테스트를 수행합니다
		if (m_beginCheck == false)
		{
			m_beginCheck = true;
			
			if (currentStage == StageEnum::Title) {
				currentStage = StageEnum::Stage;
				m_Text->ChangeRenderType(TextClass::RenderType::Stage);
			}

			else
			{
				TestIntersection();

				m_Sound->PlaySoundForSFX();
			}
			
		}
	}

	// 왼쪽 마우스 단추가 놓여 졌는지 확인하십시오
	if (m_Input->IsLeftMouseButtonDown() == false)
	{
		m_beginCheck = false;
	}


	return true;
}

bool GraphicsClass::RenderSceneToTexture()
{
	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix;

	float posX = 0;
	float posY = 0;
	float posZ = 0;

	float rotX = 0;
	float rotY = 0;
	float rotZ = 0;

	float scaleX = 0;
	float scaleY = 0;
	float scaleZ = 0;

	// 렌더링 대상을 렌더링에 맞게 설정합니다.
	m_RenderTexture->SetRenderTarget(m_D3D->GetDeviceContext());

	// 렌더링을 텍스처에 지웁니다.
	m_RenderTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 조명의 위치에 따라 조명보기 행렬을 생성합니다.
	m_Light->GenerateViewMatrix();

	// d3d 객체에서 세계 행렬을 가져옵니다.
	m_D3D->GetWorldMatrix(worldMatrix);

	// 라이트 오브젝트에서 뷰 및 정사각형 매트릭스를 가져옵니다.
	m_Light->GetViewMatrix(lightViewMatrix);
	m_Light->GetProjectionMatrix(lightProjectionMatrix);


	// 터레인 위치 설정
	m_D3D->GetWorldMatrix(worldMatrix);
	worldMatrix *= XMMatrixTranslation(-100, -3.0f, -100);

	// 지형 버퍼를 렌더링 합니다.
	m_Terrain->Render(m_D3D->GetDeviceContext());

	m_Terrain->Render(m_D3D->GetDeviceContext());
	bool result = m_DepthShader->Render(m_D3D->GetDeviceContext(), m_Terrain->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	if (!result)
	{
		return false;
	}

	for (auto m_Model : m_Models)
	{
		if (!m_Model->isAlive()) continue;

		m_D3D->GetWorldMatrix(worldMatrix);
		m_Model->GetPosition(posX, posY, posZ);
		m_Model->GetRotation(rotX, rotY, rotZ);
		m_Model->GetScale(scaleX, scaleY, scaleZ);

		worldMatrix *= XMMatrixScaling(scaleX, scaleY, scaleZ);
		worldMatrix *= XMMatrixRotationRollPitchYaw(rotX, rotY, rotZ);
		worldMatrix *= XMMatrixTranslation(posX, posY, posZ);

		m_Model->Render(m_D3D->GetDeviceContext());
		bool result = m_DepthShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		if (!result)
		{
			return false;
		}
	}

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링에 대한 렌더링을 더 이상 다시 설정하지 않습니다.
	m_D3D->SetBackBufferRenderTarget();

	// 뷰포트를 원본으로 다시 설정합니다.
	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::RenderBlackAndWhiteShadows()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMMATRIX lightViewMatrix, lightProjectionMatrix;
	float posX, posY, posZ;
	float rotX, rotY, rotZ;
	float scaleX, scaleY, scaleZ;
	bool result;

	XMMATRIX terrainMatrix;

	// 렌더링 대상을 렌더링에 맞게 설정합니다.
	m_BlackWhiteRenderTexture->SetRenderTarget(m_D3D->GetDeviceContext());

	// 렌더링을 텍스처에 지웁니다.
	m_BlackWhiteRenderTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치에 따라 뷰 행렬을 생성합니다.
	m_Camera->Render();

	// 조명의 위치에 따라 조명보기 행렬을 생성합니다.
	m_Light->GenerateViewMatrix();

	// 카메라 및 d3d 객체에서 월드, 뷰 및 투영 행렬을 가져옵니다. 
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	// 라이트 오브젝트로부터 라이트의 뷰와 투영 행렬을 가져옵니다. 
	m_Light->GetViewMatrix(lightViewMatrix);
	m_Light->GetProjectionMatrix(lightProjectionMatrix);

	m_D3D->GetWorldMatrix(terrainMatrix);
	terrainMatrix *= XMMatrixTranslation(-100, -3.0, -100);

	// 지형 버퍼를 렌더링 합니다.
	m_Terrain->Render(m_D3D->GetDeviceContext());
	result = m_ShadowShader->Render(m_D3D->GetDeviceContext(), m_Terrain->GetIndexCount(), terrainMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
		lightProjectionMatrix, m_RenderTexture->GetShaderResourceView(), m_Light->GetPosition());
	if (!result)
	{
		return false;
	}

	// 모델 렌더링
	for (auto m_Model : m_Models)
	{
		if (!m_Model->isAlive()) continue;

		m_D3D->GetWorldMatrix(worldMatrix);
		m_Model->GetPosition(posX, posY, posZ);
		m_Model->GetRotation(rotX, rotY, rotZ);
		m_Model->GetScale(scaleX, scaleY, scaleZ);

		worldMatrix *= XMMatrixScaling(scaleX, scaleY, scaleZ);
		worldMatrix *= XMMatrixRotationRollPitchYaw(rotX, rotY, rotZ);
		worldMatrix *= XMMatrixTranslation(posX, posY, posZ);

		m_Model->Render(m_D3D->GetDeviceContext());
		result = m_ShadowShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, lightViewMatrix,
			lightProjectionMatrix, m_RenderTexture->GetShaderResourceView(), m_Light->GetPosition());
		if (!result)
		{
			return false;
		}
	}

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링 대상을 더 이상 텍스처로 다시 설정하지 않습니다. 
	m_D3D->SetBackBufferRenderTarget();

	// 뷰포트를 원본으로 다시 설정합니다. 
	m_D3D->ResetViewport();

	return true;
}


bool GraphicsClass::DownSampleTexture()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	bool result;


	// 렌더링 대상을 렌더링에 맞게 설정합니다. 
	m_DownSampleTexure->SetRenderTarget(m_D3D->GetDeviceContext());

	// 렌더링을 텍스처에 지웁니다. 
	m_DownSampleTexure->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치에 따라 뷰 행렬을 생성합니다. 
	m_Camera->Render();

	// 카메라와 d3d 객체로부터 월드와 뷰 매트릭스를 얻는다. 
	m_D3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetBaseViewMatrix(baseViewMatrix);

	// 질감이 크기가 작기 때문에 렌더링에서 텍스처로 ortho 행렬을 가져옵니다. 
	m_DownSampleTexure->GetOrthoMatrix(orthoMatrix);

	// 모든 2D 렌더링을 시작하려면 Z 버퍼를 끕니다. 
	m_D3D->TurnZBufferOff();

	// 드로잉을 준비하기 위해 그래픽 파이프 라인에 작은 ortho window 버텍스와 인덱스 버퍼를 놓습니다. 
	m_SmallWindow->Render(m_D3D->GetDeviceContext());

	// 텍스처 쉐이더를 사용하여 작은 ortho 창을 렌더링하고 씬의 텍스처를 텍스처 리소스로 렌더링합니다. 
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, baseViewMatrix, orthoMatrix,
		m_BlackWhiteRenderTexture->GetShaderResourceView());
	if (!result)
	{
		return false;
	}

	// 모든 2D 렌더링이 완료되었으므로 Z 버퍼를 다시 켜십시오. 
	m_D3D->TurnZBufferOn();

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링에 대한 렌더링을 더 이상 다시 설정하지 않습니다. 
	m_D3D->SetBackBufferRenderTarget();

	// 뷰포트를 원본으로 다시 설정합니다. 
	m_D3D->ResetViewport();

	return true;
}


bool GraphicsClass::RenderHorizontalBlurToTexture()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	float screenSizeX;
	bool result;


	// 수평 블러 쉐이더에서 사용될 float에 화면 폭을 저장합니다. 
	screenSizeX = (float)(SHADOWMAP_WIDTH / 2);

	// 렌더링 대상을 렌더링에 맞게 설정합니다. 
	m_HorizontalBlurTexture->SetRenderTarget(m_D3D->GetDeviceContext());

	// 렌더링을 텍스처에 지웁니다. 
	m_HorizontalBlurTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치에 따라 뷰 행렬을 생성합니다. 
	m_Camera->Render();

	// 카메라와 d3d 객체로부터 월드와 뷰 매트릭스를 얻는다.
	m_Camera->GetBaseViewMatrix(baseViewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);

	// 텍스쳐가 다른 차원을 가지므로 렌더링에서 오쏘 (ortho) 행렬을 텍스처로 가져옵니다. 
	m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

	// 모든 2D 렌더링을 시작하려면 Z 버퍼를 끕니다. 
	m_D3D->TurnZBufferOff();

	// 드로잉을 준비하기 위해 그래픽 파이프 라인에 작은 ortho window 버텍스와 인덱스 버퍼를 놓습니다. 
	m_SmallWindow->Render(m_D3D->GetDeviceContext());

	// horizontal blur shader와 down sampled render를 사용하여 작은 ortho 윈도우를 텍스처 리소스로 렌더링합니다. 
	result = m_HorizontalBlurShader->Render(m_D3D->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, baseViewMatrix, orthoMatrix,
		m_DownSampleTexure->GetShaderResourceView(), screenSizeX);
	if (!result)
	{
		return false;
	}

	// 모든 2D 렌더링이 완료되었으므로 Z 버퍼를 다시 켜십시오.
	m_D3D->TurnZBufferOn();

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링에 대한 렌더링을 더 이상 다시 설정하지 않습니다. 
	m_D3D->SetBackBufferRenderTarget();

	// 뷰포트를 원본으로 다시 설정합니다. 
	m_D3D->ResetViewport();

	return true;
}


bool GraphicsClass::RenderVerticalBlurToTexture()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	float screenSizeY;
	bool result;


	// 수직 블러 셰이더에서 사용되는 부동 소수점에 화면 높이를 저장합니다. 
	screenSizeY = (float)(SHADOWMAP_HEIGHT / 2);

	// 렌더링 대상을 렌더링에 맞게 설정합니다. 
	m_VerticalBlurTexture->SetRenderTarget(m_D3D->GetDeviceContext());

	// 렌더링을 텍스처에 지웁니다. 
	m_VerticalBlurTexture->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치에 따라 뷰 행렬을 생성합니다. 
	m_Camera->Render();

	// 카메라와 d3d 객체로부터 월드와 뷰 매트릭스를 얻는다.
	m_Camera->GetBaseViewMatrix(baseViewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);

	// 텍스쳐가 다른 차원을 가지므로 렌더링에서 오쏘 (ortho) 행렬을 텍스처로 가져옵니다. 
	m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);

	// 모든 2D 렌더링을 시작하려면 Z 버퍼를 끕니다. 
	m_D3D->TurnZBufferOff();

	// 드로잉을 준비하기 위해 그래픽 파이프 라인에 작은 ortho window 버텍스와 인덱스 버퍼를 놓습니다. 
	m_SmallWindow->Render(m_D3D->GetDeviceContext());

	// 수직 블러 쉐이더와 수평 블러 링을 사용하여 작은 ortho 윈도우를 텍스처 리소스로 렌더합니다. 
	result = m_VerticalBlurShader->Render(m_D3D->GetDeviceContext(), m_SmallWindow->GetIndexCount(), worldMatrix, baseViewMatrix, orthoMatrix,
		m_HorizontalBlurTexture->GetShaderResourceView(), screenSizeY);
	if (!result)
	{
		return false;
	}

	// 모든 2D 렌더링이 완료되었으므로 Z 버퍼를 다시 켜십시오. 
	m_D3D->TurnZBufferOn();

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링에 대한 렌더링을 더 이상 다시 설정하지 않습니다. 
	m_D3D->SetBackBufferRenderTarget();

	// 뷰포트를 원본으로 다시 설정합니다. 
	m_D3D->ResetViewport();

	return true;
}


bool GraphicsClass::UpSampleTexture()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	bool result;


	// 렌더링 대상을 렌더링에 맞게 설정합니다. 
	m_UpSampleTexure->SetRenderTarget(m_D3D->GetDeviceContext());

	// 렌더링을 텍스처에 지웁니다. 
	m_UpSampleTexure->ClearRenderTarget(m_D3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치에 따라 뷰 행렬을 생성합니다. 
	m_Camera->Render();

	// 카메라와 d3d 객체로부터 월드와 뷰 매트릭스를 얻는다. 
	m_Camera->GetBaseViewMatrix(baseViewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);

	// 텍스쳐가 다른 차원을 가지므로 렌더링에서 오쏘 (ortho) 행렬을 텍스처로 가져옵니다.
	m_UpSampleTexure->GetOrthoMatrix(orthoMatrix);

	// 모든 2D 렌더링을 시작하려면 Z 버퍼를 끕니다. 
	m_D3D->TurnZBufferOff();

	// 그래픽 파이프 라인에 전체 화면 직교 윈도우 버텍스와 인덱스 버퍼를 배치하여 그리기를 준비합니다. 
	m_FullScreenWindow->Render(m_D3D->GetDeviceContext());

	// 텍스처 쉐이더와 텍스처 리소스에 대한 작은 크기의 최종 흐리게 렌더링을 사용하여 전체 화면 ortho 창을 렌더링합니다. 
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(), worldMatrix, baseViewMatrix, orthoMatrix,
		m_VerticalBlurTexture->GetShaderResourceView());
	if (!result)
	{
		return false;
	}

	// 모든 2D 렌더링이 완료되었으므로 Z 버퍼를 다시 켜십시오. 
	m_D3D->TurnZBufferOn();

	// 렌더링 대상을 원래의 백 버퍼로 다시 설정하고 렌더링에 대한 렌더링을 더 이상 다시 설정하지 않습니다. 
	m_D3D->SetBackBufferRenderTarget();

	// 뷰포트를 원본으로 다시 설정합니다. 
	m_D3D->ResetViewport();

	return true;
}

bool GraphicsClass::Render(float frameTime)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	XMMATRIX terrainMatrix;

	XMFLOAT3 cameraPosition;

	float posX = 0;
	float posY = 0;
	float posZ = 0;

	float rotX = 0;
	float rotY = 0;
	float rotZ = 0;

	float scaleX = 0;
	float scaleY = 0;
	float scaleZ = 0;

	XMMATRIX lightViewMatrix, lightProjectionMatrix;

	bool result;

	static float frameTimer = 0.0f;


	// 프레임 시간 카운터를 증가시킵니다.
	frameTimer += frameTime * 0.001f;
	if (frameTimer > 1000.0f)
	{
		frameTimer = 0.0f;
	}

	// 세 가지 다른 노이즈 텍스처에 대해 세 가지 스크롤 속도를 설정합니다.
	XMFLOAT3 scrollSpeeds = XMFLOAT3(1.3f, 2.1f, 2.3f);

	// 세 개의 서로 다른 노이즈 옥타브 텍스처를 만드는 데 사용할 세 개의 스케일을 설정합니다.
	XMFLOAT3 scales = XMFLOAT3(1.0f, 2.0f, 3.0f);

	// 세 가지 다른 노이즈 텍스처에 대해 세 가지 다른 x 및 y 왜곡 인수를 설정합니다.
	XMFLOAT2 distortion1 = XMFLOAT2(0.1f, 0.2f);
	XMFLOAT2 distortion2 = XMFLOAT2(0.1f, 0.3f);
	XMFLOAT2 distortion3 = XMFLOAT2(0.1f, 0.1f);

	// 텍스처 좌표 샘플링 섭동의 스케일과 바이어스.
	float distortionScale = 0.8f;
	float distortionBias = 0.5f;



	switch (currentStage)
	{
		case StageEnum::Title:
			{
				// Clear the buffers to begin the scene.
				m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
				
				// Generate the view matrix based on the camera's position.
				m_Camera->Render();

				// 조명의 위치에 따라 조명보기 행렬을 생성합니다.
				m_Light->GenerateViewMatrix();

				// Get the world, view, and projection matrices from the camera and d3d objects.
				m_D3D->GetWorldMatrix(worldMatrix);
				m_Camera->GetViewMatrix(viewMatrix);
				m_D3D->GetWorldMatrix(terrainMatrix);

				m_D3D->GetProjectionMatrix(projectionMatrix);
				m_D3D->GetOrthoMatrix(orthoMatrix);

				m_D3D->GetWorldMatrix(worldMatrix);

				worldMatrix *= XMMatrixScaling(5.5f, 5.5f, 1.f);
				m_Camera->GetBaseViewMatrix(viewMatrix);

				// 화재 투명도의 알파 블렌드를 켭니다.
				m_D3D->TurnOnAlphaBlending();

				// 정사각형 모델의 정점과 인덱스 버퍼를 그래픽 파이프 라인에 배치하여 그리기를 준비합니다.
				m_ModelForFire->Render(m_D3D->GetDeviceContext());

				// 불 쉐이더를 사용하여 사각형 모델을 렌더링합니다.
				result = m_FireShader->Render(m_D3D->GetDeviceContext(), m_ModelForFire->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
					m_ModelForFire->GetTexture1(), m_ModelForFire->GetTexture2(), m_ModelForFire->GetTexture3(), frameTimer, scrollSpeeds,
					scales, distortion1, distortion2, distortion3, distortionScale, distortionBias);
				if (!result)
				{
					return false;
				}

				m_D3D->TurnOffAlphaBlending();


				m_D3D->GetWorldMatrix(worldMatrix);
				// 화면 내 위치 변환을 막기 위해 BaseViewMatrix로 전환
				m_Camera->GetBaseViewMatrix(viewMatrix);
				// Turn off the Z buffer to begin all 2D rendering.
				m_D3D->TurnZBufferOff();

				// Turn on the alpha blending before rendering the text.
				m_D3D->TurnOnAlphaBlending();

				// 입력 객체에서 마우스의 위치를 ​​가져옵니다.
				int mouseX = 0;
				int mouseY = 0;
				m_Input->GetMouseLocation(mouseX, mouseY);

				result = m_MouseCursor->Render(m_D3D->GetDeviceContext(), mouseX, mouseY);
				// 텍스처 셰이더로 마우스 커서를 렌더링합니다.
				if (!result)
				{
					return false;
				}

				result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_MouseCursor->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_MouseCursor->GetTexture());
				if (!result)
				{
					return false;
				}

				// Render the text strings.
				result = m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
				if (!result)
				{
					return false;
				}

				// Turn off alpha blending after rendering the text.
				m_D3D->TurnOffAlphaBlending();

				// Turn the Z buffer back on now that all 2D rendering has completed.
				m_D3D->TurnZBufferOn();

				// Present the rendered scene to the screen.
				m_D3D->EndScene();

				return true;
			}

		case StageEnum::Stage:
			{
				// 먼저 장면을 텍스처로 렌더링합니다.
				result = RenderSceneToTexture();
				if (!result)
				{
					return false;
				}

				// 다음으로 그림자가있는 장면을 흑백으로 렌더링합니다. 
				result = RenderBlackAndWhiteShadows();
				if (!result)
				{
					return false;
				}

				// 그런 다음 흑백 씬 텍스처를 샘플링합니다. 
				result = DownSampleTexture();
				if (!result)
				{
					return false;
				}

				// 다운 샘플링 된 텍스처에 수평 블러를 수행합니다. 
				result = RenderHorizontalBlurToTexture();
				if (!result)
				{
					return false;
				}

				// 이제 텍스처에 수직 블러를 수행합니다. 
				result = RenderVerticalBlurToTexture();
				if (!result)
				{
					return false;
				}

				// 최종적으로 부드러운 그림자 쉐이더에서 사용할 수있는 텍스처에 대한 최종 흐린 렌더링 샘플을 샘플링합니다. 
				result = UpSampleTexture();
				if (!result)
				{
					return false;
				}

				// Clear the buffers to begin the scene.
				m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

				// Generate the view matrix based on the camera's position.
				m_Camera->Render();

				// 조명의 위치에 따라 조명보기 행렬을 생성합니다.
				m_Light->GenerateViewMatrix();

				// Get the world, view, and projection matrices from the camera and d3d objects.
				m_D3D->GetWorldMatrix(worldMatrix);
				m_Camera->GetViewMatrix(viewMatrix);
				m_D3D->GetWorldMatrix(terrainMatrix);

				m_D3D->GetProjectionMatrix(projectionMatrix);
				m_D3D->GetOrthoMatrix(orthoMatrix);

				// 라이트 오브젝트로부터 라이트의 뷰와 투영 행렬을 가져옵니다.
				m_Light->GetViewMatrix(lightViewMatrix);
				m_Light->GetProjectionMatrix(lightProjectionMatrix);


				// 카메라 위치를 얻는다.
				cameraPosition = m_Camera->GetPosition();

				// 스카이 돔을 카메라 위치를 중심으로 변환합니다.
				worldMatrix *= XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

				// 표면 컬링을 끕니다.
				m_D3D->TurnOffCulling();

				// Z 버퍼를 끈다.
				m_D3D->TurnZBufferOff();

				// 스카이 돔 셰이더를 사용하여 하늘 돔을 렌더링합니다.
				m_SkyDome->Render(m_D3D->GetDeviceContext());
				
				m_SkyDomeShader->Render(m_D3D->GetDeviceContext(), m_SkyDome->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
					m_SkyDome->GetApexColor(), m_SkyDome->GetCenterColor());

				// Z 버퍼를 다시 켭니다.
				m_D3D->TurnZBufferOn();

				// 다시 표면 컬링을 되돌립니다.
				m_D3D->TurnOnCulling();

				// 월드 행렬을 재설정합니다.
				m_D3D->GetWorldMatrix(worldMatrix);

				terrainMatrix *= XMMatrixTranslation(-100, -3.f, -100);

				// 지형 버퍼를 렌더링 합니다.
				m_Terrain->Render(m_D3D->GetDeviceContext());

				result = m_SoftShadowShader->Render(m_D3D->GetDeviceContext(), m_Terrain->GetIndexCount(), terrainMatrix, viewMatrix, projectionMatrix,
					m_Terrain->GetTexture(), m_UpSampleTexure->GetShaderResourceView(), m_Light->GetPosition(),
					m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(), m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
				if (!result)
				{
					return false;
				}


				m_D3D->GetWorldMatrix(worldMatrix);

				m_Foliage->GetPosition(posX, posY, posZ);
				worldMatrix = XMMatrixTranslation(posX, posY, posZ);

				// 깊이 버퍼 끄기
				m_D3D->TurnZBufferOff();

				// 투명 켜기
				m_D3D->TurnOnAlphaBlending();

				// 단풍을 렌더링합니다.
				m_Foliage->Render(m_D3D->GetDeviceContext());
				m_FoliageShader->Render(m_D3D->GetDeviceContext(), m_Foliage->GetVertexCount(), m_Foliage->GetInstanceCount(), viewMatrix, projectionMatrix, m_Foliage->GetTexture());

				// 투명 끄기
				m_D3D->TurnOffAlphaBlending();

				// 2D 렌더링이 끝나면 깊이 버퍼 다시 켜기
				m_D3D->TurnZBufferOn();


				// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
				for (auto m_Model : m_Models)
				{
					if (!m_Model->isAlive()) continue;

					m_D3D->GetWorldMatrix(worldMatrix);
					m_Model->GetPosition(posX, posY, posZ);
					m_Model->GetRotation(rotX, rotY, rotZ);
					m_Model->GetScale(scaleX, scaleY, scaleZ);

					worldMatrix *= XMMatrixScaling(scaleX, scaleY, scaleZ);
					worldMatrix *= XMMatrixRotationRollPitchYaw(rotX, rotY, rotZ);
					worldMatrix *= XMMatrixTranslation(posX, posY, posZ);


					m_Model->Render(m_D3D->GetDeviceContext());

					result = m_SoftShadowShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
						m_Model->GetTexture(), m_UpSampleTexure->GetShaderResourceView(), m_Light->GetDirection(),
						m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetPosition(), m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
					if (!result)
					{
						return false;
					}

				}


				m_D3D->GetWorldMatrix(worldMatrix);
				// 화면 내 위치 변환을 막기 위해 BaseViewMatrix로 전환
				m_Camera->GetBaseViewMatrix(viewMatrix);
				// Turn off the Z buffer to begin all 2D rendering.
				m_D3D->TurnZBufferOff();

				// Turn on the alpha blending before rendering the text.
				m_D3D->TurnOnAlphaBlending();

				// Render the text strings.
				result = m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
				if (!result)
				{
					return false;
				}


				for (auto m_Crosshair : m_Crosshairs) {

					m_D3D->GetWorldMatrix(worldMatrix);
					m_Crosshair->GetPosition(posX, posY, posZ);
					m_Crosshair->GetScale(scaleX, scaleY, scaleZ);

					worldMatrix *= XMMatrixScaling(scaleX, scaleY, scaleZ);
					worldMatrix *= XMMatrixTranslation(posX, posY, posZ);

					// Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
					result = m_Crosshair->Render(m_D3D->GetDeviceContext(), 200, 200);
					if (!result)
					{
						return false;
					}

					// Render the bitmap with the texture shader.
					result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Crosshair->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Crosshair->GetTexture());
					if (!result)
					{
						return false;
					}
				}
				
				// Turn off alpha blending after rendering the text.
				m_D3D->TurnOffAlphaBlending();

				// Turn the Z buffer back on now that all 2D rendering has completed.
				m_D3D->TurnZBufferOn();

				// Present the rendered scene to the screen.
				m_D3D->EndScene();

				return true;
			}

		default:
			break;
	}

	return true;
}

CameraClass* GraphicsClass::GetCam()
{
	return m_Camera;
}

void GraphicsClass::TestIntersection()
{
	XMMATRIX projectionMatrix, viewMatrix, inverseViewMatrix, worldMatrix, translateMatrix, inverseWorldMatrix;
	XMFLOAT3 direction, origin, rayOrigin, rayDirection;

	// 마우스 커서 좌표를 -1에서 +1 범위로 이동합니다
	float pointX = ((2.0f * (float)m_screenWidth / 2) / (float)m_screenWidth) - 1.0f;
	float pointY = (((2.0f * (float)m_screenHeight / 2) / (float)m_screenHeight) - 1.0f) * -1.0f;

	// 뷰포트의 종횡비를 고려하여 투영 행렬을 사용하여 점을 조정합니다
	m_D3D->GetProjectionMatrix(projectionMatrix);

	XMFLOAT3X3 projectionMatrix4;
	XMStoreFloat3x3(&projectionMatrix4, projectionMatrix);

	pointX = pointX / projectionMatrix4._11;
	pointY = pointY / projectionMatrix4._22;

	// 뷰 행렬의 역함수를 구합니다.
	m_Camera->GetViewMatrix(viewMatrix);
	inverseViewMatrix = XMMatrixInverse(nullptr, viewMatrix);

	XMFLOAT3X3 inverseViewMatrix4;
	XMStoreFloat3x3(&inverseViewMatrix4, inverseViewMatrix);

	// 뷰 공간에서 피킹 레이의 방향을 계산합니다.
	direction.x = (pointX * inverseViewMatrix4._11) + (pointY * inverseViewMatrix4._21) + inverseViewMatrix4._31;
	direction.y = (pointX * inverseViewMatrix4._12) + (pointY * inverseViewMatrix4._22) + inverseViewMatrix4._32;
	direction.z = (pointX * inverseViewMatrix4._13) + (pointY * inverseViewMatrix4._23) + inverseViewMatrix4._33;

	// 카메라의 위치 인 picking ray의 원점을 가져옵니다.
	origin = m_Camera->GetPosition();

	for (int i = 0; i < m_Models.size(); i++) {
		// 세계 행렬을 가져와 구의 위치로 변환합니다.
		m_D3D->GetWorldMatrix(worldMatrix);
		
		float posX, posY, posZ;
		m_Models[i]->GetPosition(posX, posY, posZ);

		translateMatrix = XMMatrixTranslation(posX, posY, posZ);
		worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

		// 이제 번역 된 행렬의 역함수를 구하십시오.
		inverseWorldMatrix = XMMatrixInverse(nullptr, worldMatrix);

		// 이제 광선 원점과 광선 방향을 뷰 공간에서 월드 공간으로 변환합니다.
		XMStoreFloat3(&rayOrigin, XMVector3TransformCoord(XMVectorSet(origin.x, origin.y, origin.z, 0.0f), inverseWorldMatrix));
		XMStoreFloat3(&direction, XMVector3TransformNormal(XMVectorSet(direction.x, direction.y, direction.z, 0.0f), inverseWorldMatrix));

		// 광선 방향을 표준화합니다.
		XMStoreFloat3(&rayDirection, XMVector3Normalize(XMVectorSet(direction.x, direction.y, direction.z, 0.0f)));

		// 이제 광선 구 교차 테스트를 수행하십시오.
		if (RaySphereIntersect(rayOrigin, rayDirection, 1.0f) == true)
		{
			// 교차하는 경우 화면에 표시되는 텍스트 문자열에서 교차로를 "yes"로 설정합니다.
			if (m_Models[i]->isAlive()) {
				m_Models[i]->SetDie();
				wholeObj--;
				wholePoly -= m_Models[i]->CountPolygons();
			}
			break;
		}
		else
		{
			// 그렇지 않으면 "No"로 교차를 설정하십시오.

		}
	}
}


bool GraphicsClass::RaySphereIntersect(XMFLOAT3 rayOrigin, XMFLOAT3 rayDirection, float radius)
{
	// a, b 및 c 계수를 계산합니다.
	float a = (rayDirection.x * rayDirection.x) + (rayDirection.y * rayDirection.y) + (rayDirection.z * rayDirection.z);
	float b = ((rayDirection.x * rayOrigin.x) + (rayDirection.y * rayOrigin.y) + (rayDirection.z * rayOrigin.z)) * 2.0f;
	float c = ((rayOrigin.x * rayOrigin.x) + (rayOrigin.y * rayOrigin.y) + (rayOrigin.z * rayOrigin.z)) - (radius * radius);

	// 결과값을 얻는다
	float discriminant = (b * b) - (4 * a * c);

	// 결과값이 음수이면 피킹 선이 구를 벗어난 것입니다. 그렇지 않으면 구를 선택합니다.
	if (discriminant < 0.0f)
	{
		return false;
	}

	return true;
}