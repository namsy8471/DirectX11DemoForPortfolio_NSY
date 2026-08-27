////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////

#include "graphicsclass.h"
#include "bitmapclass.h"
#include "cameraclass.h"
#include "CollisionHelpers.h"
#include "d3dclass.h"
#include "FireShaderClass.h"
#include "foliageclass.h"
#include "foliageshaderclass.h"
#include "inputclass.h"
#include "LightClass.h"
#include "modelclass.h"
#include "ModelClassForNoiseFilter.h"
#include "SkyDomeClass.h"
#include "SkyDomeShaderClass.h"
#include "SoundClass.h"
#include "TerrainClass.h"
#include "textclass.h"
#include "textureshaderclass.h"
#include "Engine/Input/FirstPersonCameraController.h"
#include "Engine/Rendering/SoftShadowPipeline.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Core/ScopeExit.h"

#include <utility>

using namespace DirectX;

GraphicsClass::GraphicsClass(Engine::Scene::SceneDefinition sceneDefinition)
	: m_sceneDefinition(std::move(sceneDefinition))
{
}
GraphicsClass::~GraphicsClass() = default;

bool GraphicsClass::Initialize(const Engine::NativeWindow& window)
{
	return InitializeResources(window.width, window.height, window.handle, window.instance);
}

bool GraphicsClass::InitializeResources(int screenWidth, int screenHeight, HWND hwnd, HINSTANCE hInstance)
{
	bool result;

	XMMATRIX baseViewMatrix;

	// 화면 너비와 높이를 저장합니다.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Create the Direct3D object.
	m_d3d = Engine::MakeScopedResource<D3DClass>();
	if(!m_d3d)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_d3d->Initialize(screenWidth, screenHeight, RenderSettings::Vsync, hwnd, RenderSettings::FullScreen, RenderSettings::ScreenDepth, RenderSettings::ScreenNear);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}



	// Create the camera object.
	m_camera = std::make_unique<CameraClass>();
	if(!m_camera)
	{
		return false;
	}

	// 카메라 포지션을 설정한다
	m_camera->SetPosition(0.0f, 0.0f, -10.0f);

	result = m_camera->Initialize();
	if (!result)
	{
		return false;
	}
	
	// 입력 개체를 만듭니다.
	m_input = Engine::MakeScopedResource<InputClass>();
	if (!m_input)
	{
		return false;
	}

	// 입력 개체를 초기화합니다.
	result = m_input->Initialize(hInstance, hwnd, screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}
	m_cameraController = std::make_unique<Engine::FirstPersonCameraController>();

	// Create the model object.
	for (const auto& definition : m_sceneDefinition.gameObjects)
	{
		auto gameObject = std::make_unique<Engine::Scene::GameObject>();
		result = gameObject->InitializeModel(
			hwnd,
			m_d3d->GetDevice(),
			definition.modelPath.c_str(),
			definition.texturePath.c_str());
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}

		m_objectCount++;
		m_polygonCount += static_cast<unsigned int>(gameObject->GetPolygonCount());
		gameObject->SetTransform(definition.transform);
		m_gameObjects.push_back(std::move(gameObject));
	}


	// 불 효과 모델 객체 생성
	m_fireModel = Engine::MakeScopedResource<ModelClassForNoiseFilter>();
	if (!m_fireModel)
	{
		return false;
	}

	// 모델 객체 초기화
	if (!m_fireModel->Initialize(
		hwnd,
		m_d3d->GetDevice(),
		m_sceneDefinition.fire.modelPath.c_str(),
		m_sceneDefinition.fire.fireTexturePath.c_str(),
		m_sceneDefinition.fire.noiseTexturePath.c_str(),
		m_sceneDefinition.fire.alphaTexturePath.c_str()))
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}
	m_objectCount++;
	m_polygonCount += static_cast<unsigned int>(m_fireModel->GetIndexCount() / 3);

	// 파이어 쉐이더 객체를 생성한다.
	m_fireShader = Engine::MakeScopedResource<FireShaderClass>();
	if (!m_fireShader)
	{
		return false;
	}

	// 화재 쉐이더 객체를 초기화합니다.
	if (!m_fireShader->Initialize(m_d3d->GetDevice(), hwnd))
	{
		MessageBox(hwnd, L"Could not initialize the fire shader object.", L"Error", MB_OK);
		return false;
	}

	m_textureShader = Engine::MakeScopedResource<TextureShaderClass>();
	if (!m_textureShader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_textureShader->Initialize(m_d3d->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	for (int i = 0; i < 4; i++)
	{
		m_crosshairs.push_back(Engine::MakeScopedResource<BitmapClass>());
		if (!m_crosshairs[i])
		{
			return false;
		}

		result = m_crosshairs[i]->Initialize(
			m_d3d->GetDevice(),
			screenWidth,
			screenHeight,
			m_sceneDefinition.ui.crosshairTexturePath.c_str(),
			256,
			256);
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the Crosshair bitmap object.", L"Error", MB_OK);
			return false;
		}

		m_objectCount++;
	}

	m_crosshairs[0]->SetPosition(-5, 0.1f, 0);
	m_crosshairs[1]->SetPosition(5, 0.1f, 0);
	m_crosshairs[2]->SetPosition(0.1f, 5, 0);
	m_crosshairs[3]->SetPosition(0.1f, -5, 0);

	m_crosshairs[0]->SetScale(0.01f, 0.005f, 1);
	m_crosshairs[1]->SetScale(0.01f, 0.005f, 1);
	m_crosshairs[2]->SetScale(0.008f, 0.01f, 1);
	m_crosshairs[3]->SetScale(0.008f, 0.01f, 1);

	m_mouseCursor = Engine::MakeScopedResource<BitmapClass>();
	if (!m_mouseCursor)
	{
		return false;
	}

	m_objectCount++;

	result = m_mouseCursor->Initialize(
		m_d3d->GetDevice(),
		screenWidth,
		screenHeight,
		m_sceneDefinition.ui.cursorTexturePath.c_str(),
		32,
		32);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}

	m_camera->UpdateViewMatrix();
	m_camera->GetViewMatrix(baseViewMatrix);

	// Create the text object.
	m_text = Engine::MakeScopedResource<TextClass>();
	if (!m_text)
	{
		return false;
	}

	// Initialize the text object.
	result = m_text->Initialize(m_d3d->GetDevice(), m_d3d->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}


	// 지형 객체를 생성합니다.
	m_terrain = Engine::MakeScopedResource<TerrainClass>();
	if (!m_terrain)
	{
		return false;
	}

	// 지형 객체를 초기화 합니다.
	result = m_terrain->Initialize(
		m_d3d->GetDevice(),
		m_sceneDefinition.terrain.heightMapPath.c_str(),
		m_sceneDefinition.terrain.texturePath.c_str());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
		return false;
	}
	m_terrain->SetWorldOffset(m_sceneDefinition.terrain.worldOffset);

	// 조명 객체를 생성합니다.
	m_light = std::make_unique<LightClass>();
	if (!m_light)
	{
		return false;
	}

	// 조명 객체를 초기화 합니다.
	//m_light->SetAmbientColor(XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f));
	m_light->SetAmbientColor(XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f));
	m_light->SetDiffuseColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_light->SetLookAt(XMFLOAT3(0.0f, 0.0f, 10.0f));
	m_light->SetSpecularColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_light->SetSpecularPower(1000.f);
	m_light->GenerateProjectionMatrix(RenderSettings::ScreenDepth, RenderSettings::ScreenNear);


	Engine::Rendering::SoftShadowPipelineConfig shadowPipelineConfig{};
	shadowPipelineConfig.shadowMapWidth = m_sceneDefinition.shadowMap.width;
	shadowPipelineConfig.shadowMapHeight = m_sceneDefinition.shadowMap.height;
	shadowPipelineConfig.screenDepth = RenderSettings::ScreenDepth;
	shadowPipelineConfig.screenNear = RenderSettings::ScreenNear;

	m_softShadowPipeline = std::make_unique<Engine::Rendering::SoftShadowPipeline>();
	if (!m_softShadowPipeline->Initialize(m_d3d->GetDevice(), hwnd, shadowPipelineConfig))
	{
		return false;
	}

	// 스카이 돔 객체를 생성합니다.
	m_skyDome = Engine::MakeScopedResource<SkyDomeClass>();
	if (!m_skyDome)
	{
		return false;
	}

	// 스카이 돔 객체를 초기화 합니다.
	result = m_skyDome->Initialize(m_d3d->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the sky dome object.", L"Error", MB_OK);
		return false;
	}
	else{
		m_objectCount++;
		m_polygonCount += static_cast<unsigned int>(m_skyDome->GetIndexCount() / 3);
	}

	// 스카이 돔 쉐이더 객체를 생성합니다.
	m_skyDomeShader = Engine::MakeScopedResource<SkyDomeShaderClass>();
	if (!m_skyDomeShader)
	{
		return false;
	}

	// 스카이 돔 쉐이더 객체를 초기화 합니다.
	result = m_skyDomeShader->Initialize(m_d3d->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the sky dome shader object.", L"Error", MB_OK);
		return false;
	}

	// 단풍 객체를 생성합니다.
	m_foliage = Engine::MakeScopedResource<FoliageClass>();
	if (!m_foliage)
	{
		return false;
	}

	// 단풍 객체를 초기화 합니다.
	result = m_foliage->Initialize(
		m_d3d->GetDevice(),
		m_sceneDefinition.foliage.texturePath.c_str(),
		m_sceneDefinition.foliage.count);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the foliage object.", L"Error", MB_OK);
		return false;
	}


	// 단풍 쉐이더 개체를 만듭니다.
	m_foliageShader = Engine::MakeScopedResource<FoliageShaderClass>();
	if (!m_foliageShader)
	{
		return false;
	}

	// 단풍 쉐이더 개체를 초기화합니다.
	result = m_foliageShader->Initialize(m_d3d->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the foliage shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the sound object.
	m_sound = Engine::MakeScopedResource<SoundClass>();
	if (!m_sound)
	{
		return false;
	}

	// Initialize the sound object.
	if (!m_sound->Initialize(
		hwnd,
		m_sceneDefinition.audio.startupSfxPath.c_str(),
		m_sceneDefinition.audio.bgmPath.c_str(),
		m_sceneDefinition.audio.actionSfxPath.c_str()))
	{
		MessageBox(hwnd, L"Could not initialize Direct Sound.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void GraphicsClass::Shutdown() noexcept
{
	// Explicit ordering keeps every device-dependent object alive until it has
	// released its own GPU resources. Repeated calls are safe.
	m_foliageShader.reset();
	m_foliage.reset();
	m_crosshairs.clear();
	m_fireShader.reset();
	m_fireModel.reset();
	m_mouseCursor.reset();
	m_sound.reset();
	m_cameraController.reset();
	m_input.reset();
	m_softShadowPipeline.reset();
	m_skyDomeShader.reset();
	m_skyDome.reset();
	m_light.reset();
	m_terrain.reset();
	m_text.reset();
	m_textureShader.reset();
	m_gameObjects.clear();
	m_camera.reset();
	m_d3d.reset();

	m_objectCount = 0;
	m_polygonCount = 0;
}


Engine::UpdateResult GraphicsClass::Update(const Engine::FrameContext& frame)
{
	return UpdateScene(frame);
}

bool GraphicsClass::Render(const Engine::FrameContext& frame)
{
	if (!RenderScene(frame.deltaMilliseconds))
	{
		return false;
	}
	return m_d3d->EndScene();
}

Engine::UpdateResult GraphicsClass::UpdateScene(const Engine::FrameContext& frame)
{
	const float frameTime = frame.deltaMilliseconds;
	const Engine::UpdateResult inputResult = HandleInput(frameTime);
	if (inputResult != Engine::UpdateResult::Continue)
	{
		return inputResult;
	}

	switch (m_stage)
	{
	case Stage::Title:
		if (!m_text->SetGoalDescription(m_d3d->GetDeviceContext()))
		{
			return Engine::UpdateResult::Failure;
		}
		break;

	case Stage::Hunt:
		m_sound->PlaySoundForBGM();

		if (!m_text->SetFps(frame.fps, m_d3d->GetDeviceContext()))
		{
			return Engine::UpdateResult::Failure;
		}

		if (!m_text->SetCpu(frame.cpuPercentage, m_d3d->GetDeviceContext()))
		{
			return Engine::UpdateResult::Failure;
		}

		if (!m_text->SetPolygons(m_polygonCount, m_d3d->GetDeviceContext()))
		{
			return Engine::UpdateResult::Failure;
		}

		if (!m_text->SetObject(m_objectCount, m_d3d->GetDeviceContext()))
		{
			return Engine::UpdateResult::Failure;
		}

		if (!m_text->SetSizeOfScreen(m_d3d->GetDeviceContext()))
		{
			return Engine::UpdateResult::Failure;
		}
		break;

	default:
		break;
	}
	
	m_lightPositionX += 0.01f * frameTime;
	if (m_lightPositionX > 10.0f)
	{
		m_lightPositionX = -10.0f;
	}
	m_light->SetPosition(XMFLOAT3(m_lightPositionX, 10.0f, -5.0f));

	if (m_stage == Stage::Hunt)
	{
		XMFLOAT3 cameraPosition = m_camera->GetPosition();
		const float terrainHeight = m_terrain->GetHeight(cameraPosition.x, cameraPosition.z);
		const float cameraHeightOffset = 2.0f;
		const float desiredCameraHeight = terrainHeight + cameraHeightOffset;
		m_camera->SetPositionY(desiredCameraHeight);
		m_camera->UpdateViewMatrix();
		cameraPosition.y = desiredCameraHeight;

		if (!m_foliage->Frame(
			cameraPosition,
			m_d3d->GetDeviceContext(),
			frameTime * 0.01f,
			m_terrain.get()))
		{
			return Engine::UpdateResult::Failure;
		}
	}

	const Engine::Input::InputSnapshot& input = m_input->GetSnapshot();
	if (input.primaryActionDown && !m_primaryActionWasDown)
	{
		if (m_stage == Stage::Title)
		{
			m_stage = Stage::Hunt;
			m_text->ChangeRenderType(TextClass::RenderType::Stage);
			m_sound->PlaySoundForBGM();
		}
		else
		{
			TestIntersection();
			m_sound->PlaySoundForSFX();
		}
	}
	m_primaryActionWasDown = input.primaryActionDown;

	return Engine::UpdateResult::Continue;
}

Engine::UpdateResult GraphicsClass::HandleInput(float frameTime)
{
	if (!m_input->Frame(frameTime))
	{
		return Engine::UpdateResult::Failure;
	}

	const Engine::Input::InputSnapshot& input = m_input->GetSnapshot();
	if (m_stage == Stage::Hunt)
	{
		m_cameraController->Update(*m_camera, input, frameTime);
	}

	if (input.escapePressed)
	{
		return Engine::UpdateResult::ExitRequested;
	}

	return Engine::UpdateResult::Continue;
}

bool GraphicsClass::RenderScene(float frameTime)
{
	[[maybe_unused]] auto restorePipeline = Engine::MakeScopeExit([this]() noexcept {
		m_d3d->TurnOffAlphaBlending();
		m_d3d->TurnZBufferOn();
		m_d3d->TurnOnCulling();
		m_d3d->DisableDepthTestingWithoutWrites();
	});

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	XMMATRIX terrainMatrix;

	XMFLOAT3 cameraPosition;

	float posX = 0;
	float posY = 0;
	float posZ = 0;

	float scaleX = 0;
	float scaleY = 0;
	float scaleZ = 0;

	XMMATRIX lightViewMatrix, lightProjectionMatrix;

	bool result;

	// 프레임 시간 카운터를 증가시킵니다.
	m_fireAnimationTime += frameTime * 0.001f;
	if (m_fireAnimationTime > 1000.0f)
	{
		m_fireAnimationTime = 0.0f;
	}

	// 세 가지 다른 노이즈 텍스처에 대해 세 가지 스크롤 속도를 설정합니다.
	XMFLOAT3 scrollSpeeds = XMFLOAT3(1.3f, 2.1f, 2.3f);

	// 세 개의 서로 같은 노이즈 옥타브 텍스처를 만드는 데 사용할 세 개의 스케일을 설정합니다.
	XMFLOAT3 scales = XMFLOAT3(1.0f, 2.0f, 3.0f);

	// 세 가지 다른 노이즈 텍스처에 대해 세 가지 다른 x 및 y 왜곡 인수를 설정합니다.
	XMFLOAT2 distortion1 = XMFLOAT2(0.1f, 0.2f);
	XMFLOAT2 distortion2 = XMFLOAT2(0.1f, 0.3f);
	XMFLOAT2 distortion3 = XMFLOAT2(0.1f, 0.1f);

	// 텍스처 좌표 샘플링 섭동의 스케일과 바이어스.
	float distortionScale = 0.8f;
	float distortionBias = 0.5f;



	switch (m_stage)
	{
		case Stage::Title:
			{
				// Clear the buffers to begin the scene.
				m_d3d->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
				
				// Generate the view matrix based on the camera's position.
				m_camera->UpdateViewMatrix();

				// 조명의 위치에 따라 조명보기 행렬을 생성합니다.
				m_light->GenerateViewMatrix();

				// Get the world, view, and projection matrices from the camera and d3d objects.
				m_d3d->GetWorldMatrix(worldMatrix);
				m_camera->GetViewMatrix(viewMatrix);
				m_d3d->GetWorldMatrix(terrainMatrix);

				m_d3d->GetProjectionMatrix(projectionMatrix);
				m_d3d->GetOrthoMatrix(orthoMatrix);

				m_d3d->GetWorldMatrix(worldMatrix);

				const auto& fireScale = m_sceneDefinition.fire.scale;
				worldMatrix *= XMMatrixScaling(fireScale.x, fireScale.y, fireScale.z);
				m_camera->GetBaseViewMatrix(viewMatrix);

				// 화재 투명도의 알파 블렌드를 켭니다.
				m_d3d->TurnOnAlphaBlending();

				// 정사각형 모델의 정점과 인덱스 버퍼를 그래픽 파이프 라인에 배치하여 그리기를 준비합니다.
				m_fireModel->Render(m_d3d->GetDeviceContext());

				// 불 쉐이더를 사용하여 사각형 모델을 렌더링합니다.
				result = m_fireShader->Render(m_d3d->GetDeviceContext(), m_fireModel->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
					m_fireModel->GetTexture1(), m_fireModel->GetTexture2(), m_fireModel->GetTexture3(), m_fireAnimationTime, scrollSpeeds,
					scales, distortion1, distortion2, distortion3, distortionScale, distortionBias);
				if (!result)
				{
					return false;
				}

				m_d3d->TurnOffAlphaBlending();


				m_d3d->GetWorldMatrix(worldMatrix);
				// 화면 내 위치 변환을 막기 위해 BaseViewMatrix로 전환
				m_camera->GetBaseViewMatrix(viewMatrix);
				// Turn off the Z buffer to begin all 2D rendering.
				m_d3d->TurnZBufferOff();

				// Turn on the alpha blending before rendering the text.
				m_d3d->TurnOnAlphaBlending();

				// 입력 OBJECT에서 마우스의 위치를 ​​가져옵니다.
				int mouseX = 0;
				int mouseY = 0;
				m_input->GetMouseLocation(mouseX, mouseY);

				result = m_mouseCursor->Render(m_d3d->GetDeviceContext(), mouseX, mouseY);
				// 텍스처 셰이더로 마우스 커서를 렌더링합니다.
				if (!result)
				{
					return false;
				}

				result = m_textureShader->Render(m_d3d->GetDeviceContext(), m_mouseCursor->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_mouseCursor->GetTexture());
				if (!result)
				{
					return false;
				}

				// Render the text strings.
				result = m_text->Render(m_d3d->GetDeviceContext(), worldMatrix, orthoMatrix);
				if (!result)
				{
					return false;
				}

				// Turn off alpha blending after rendering the text.
				m_d3d->TurnOffAlphaBlending();

				// Restore the state before the outer application presents the frame.
				m_d3d->TurnZBufferOn();

				return true;
			}

		case Stage::Hunt:
			{
				const Engine::Rendering::SoftShadowPipelineFrame softShadowFrame{
					*m_d3d,
					*m_camera,
					*m_light,
					*m_terrain,
					m_gameObjects,
					*m_textureShader,
					m_sceneDefinition
				};
				if (!m_softShadowPipeline->Execute(softShadowFrame))
				{
					return false;
				}

				// Clear the buffers to begin the scene.
				m_d3d->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

				// Generate the view matrix based on the camera's position.
				m_camera->UpdateViewMatrix();

				// 조명의 위치에 따라 조명보기 행렬을 생성합니다.
				m_light->GenerateViewMatrix();

				// Get the world, view, and projection matrices from the camera and d3d objects.
				m_d3d->GetWorldMatrix(worldMatrix);
				m_camera->GetViewMatrix(viewMatrix);
				m_d3d->GetWorldMatrix(terrainMatrix);

				m_d3d->GetProjectionMatrix(projectionMatrix);
				m_d3d->GetOrthoMatrix(orthoMatrix);

				// 라이트 오브젝트로부터 라이트의 뷰와 투영 행렬을 가져옵니다.
				m_light->GetViewMatrix(lightViewMatrix);
				m_light->GetProjectionMatrix(lightProjectionMatrix);


				// 카메라 위치를 얻는다.
				cameraPosition = m_camera->GetPosition();

				// 스카이 돔을 카메라 위치를 중심으로 변환합니다.
				worldMatrix *= XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

				// 표면 컬링을 끕니다.
				m_d3d->TurnOffCulling();

				// Z 버퍼를 끈다.
				m_d3d->TurnZBufferOff();

				// 스카이 돔 셰이더를 사용하여 하늘 돔을 렌더링합니다.
				m_skyDome->Render(m_d3d->GetDeviceContext());
				
				if (!m_skyDomeShader->Render(
					m_d3d->GetDeviceContext(),
					m_skyDome->GetIndexCount(),
					worldMatrix,
					viewMatrix,
					projectionMatrix,
					m_skyDome->GetApexColor(),
					m_skyDome->GetCenterColor()))
				{
					return false;
				}

				// Z 버퍼를 다시 켭니다.
				m_d3d->TurnZBufferOn();

				// 다시 표면 컬링을 되돌립니다.
				m_d3d->TurnOnCulling();

				// 월드 행렬을 재설정합니다.
				m_d3d->GetWorldMatrix(worldMatrix);

				const auto& terrainOffset = m_sceneDefinition.terrain.worldOffset;
				terrainMatrix *= XMMatrixTranslation(terrainOffset.x, terrainOffset.y, terrainOffset.z);

				// 지형 버퍼를 렌더링 합니다.
				m_terrain->Render(m_d3d->GetDeviceContext());

				result = m_softShadowPipeline->RenderSurface(
					m_d3d->GetDeviceContext(),
					m_terrain->GetIndexCount(),
					terrainMatrix,
					viewMatrix,
					projectionMatrix,
					m_terrain->GetTexture(),
					*m_light);
				if (!result)
				{
					return false;
				}


				m_d3d->GetWorldMatrix(worldMatrix);

				// ✅ 깊이 테스트 ON, 깊이 쓰기 OFF로 설정
				// 지형에 의해 가려진 풀은 렌더링되지 않지만, 풀끼리는 올바르게 블렌딩됨
				m_d3d->EnableDepthTestingWithoutWrites();

				m_foliage->GetPosition(posX, posY, posZ);
				worldMatrix = XMMatrixTranslation(posX, posY, posZ);

				// 투명 켜기
				m_d3d->TurnOnAlphaBlending();

				// 단풍을 렌더링합니다.
				m_foliage->Render(m_d3d->GetDeviceContext());
				if (!m_foliageShader->Render(
					m_d3d->GetDeviceContext(),
					m_foliage->GetVertexCount(),
					m_foliage->GetInstanceCount(),
					viewMatrix,
					projectionMatrix,
					m_foliage->GetTexture()))
				{
					return false;
				}

				// 투명 끄기
				m_d3d->TurnOffAlphaBlending();

				// 일반 깊이 스텐실 상태로 복원
				m_d3d->DisableDepthTestingWithoutWrites();
	


				// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
				for (const auto& model : m_gameObjects)
				{
					if (!model->IsActive()) continue;

					worldMatrix = model->GetWorldMatrix();


					model->Render(m_d3d->GetDeviceContext());

					result = m_softShadowPipeline->RenderSurface(
						m_d3d->GetDeviceContext(),
						model->GetIndexCount(),
						worldMatrix,
						viewMatrix,
						projectionMatrix,
						model->GetTexture(),
						*m_light);
					if (!result)
					{
						return false;
					}

				}


				m_d3d->GetWorldMatrix(worldMatrix);
				// 화면 내 위치 변환을 막기 위해 BaseViewMatrix로 전환
				m_camera->GetBaseViewMatrix(viewMatrix);
				// Turn off the Z buffer to begin all 2D rendering.
				m_d3d->TurnZBufferOff();

				// Turn on the alpha blending before rendering the text.
				m_d3d->TurnOnAlphaBlending();

				// Render the text strings.
				result = m_text->Render(m_d3d->GetDeviceContext(), worldMatrix, orthoMatrix);
				if (!result)
				{
					return false;
				}


				for (const auto& crosshair : m_crosshairs) {

					m_d3d->GetWorldMatrix(worldMatrix);
					crosshair->GetPosition(posX, posY, posZ);
					crosshair->GetScale(scaleX, scaleY, scaleZ);

					worldMatrix *= XMMatrixScaling(scaleX, scaleY, scaleZ);
					worldMatrix *= XMMatrixTranslation(posX, posY, posZ);

					// Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
					result = crosshair->Render(m_d3d->GetDeviceContext(), 200, 200);
					if (!result)
					{
						return false;
					}

					// Render the bitmap with the texture shader.
					result = m_textureShader->Render(m_d3d->GetDeviceContext(), crosshair->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, crosshair->GetTexture());
					if (!result)
					{
						return false;
					}
				}
				
				// Turn off alpha blending after rendering the text.
				m_d3d->TurnOffAlphaBlending();

				// Turn the Z buffer back on now that all 2D rendering has completed.
				m_d3d->TurnZBufferOn();

				return true;
			}

		default:
			break;
	}

	return true;
}

void GraphicsClass::TestIntersection()
{
	XMMATRIX projectionMatrix, viewMatrix;
	XMFLOAT3 rayOrigin, rayDirection;

	// 마우스 커서 좌표를 -1에서 +1 범위로 이동 (화면 중앙 기준)
	float pointX = ((2.0f * static_cast<float>(m_screenWidth) / 2) / static_cast<float>(m_screenWidth)) - 1.0f;
	float pointY = (((2.0f * static_cast<float>(m_screenHeight) / 2) / static_cast<float>(m_screenHeight)) - 1.0f) * -1.0f;

	// 투영 행렬을 가져옴
	m_d3d->GetProjectionMatrix(projectionMatrix);

	// 투영 행렬의 역행렬을 사용하여 뷰 공간으로 변환
	XMMATRIX invProjection = XMMatrixInverse(nullptr, projectionMatrix);
	XMVECTOR nearPoint = XMVectorSet(pointX, pointY, 0.0f, 1.0f);
	XMVECTOR farPoint = XMVectorSet(pointX, pointY, 1.0f, 1.0f);

	nearPoint = XMVector3TransformCoord(nearPoint, invProjection);
	farPoint = XMVector3TransformCoord(farPoint, invProjection);

	// 뷰 행렬의 역행렬을 구함
	m_camera->GetViewMatrix(viewMatrix);
	XMMATRIX invView = XMMatrixInverse(nullptr, viewMatrix);

	// 월드 공간으로 변환
	nearPoint = XMVector3TransformCoord(nearPoint, invView);
	farPoint = XMVector3TransformCoord(farPoint, invView);

	// Ray 원점과 방향 계산
	XMStoreFloat3(&rayOrigin, nearPoint);
	XMVECTOR direction = XMVector3Normalize(farPoint - nearPoint);
	XMStoreFloat3(&rayDirection, direction);

	// 각 모델과 충돌 검사
	float closestDistance = FLT_MAX;
	int hitModelIndex = -1;

	for (int i = 0; i < static_cast<int>(m_gameObjects.size()); i++)
	{
		if (!m_gameObjects[i]->IsActive()) continue;

		// 월드 공간 AABB 가져오기
		CollisionHelpers::AABB worldAABB = m_gameObjects[i]->GetWorldAABB();

		// Ray-AABB 교차 테스트
		float distance;
		if (CollisionHelpers::RayAABBIntersect(rayOrigin, rayDirection, worldAABB, &distance))
		{
			// 가장 가까운 모델 찾기
			if (distance < closestDistance && distance >= 0.0f)
			{
				closestDistance = distance;
				hitModelIndex = i;
			}
		}
	}

	// 충돌한 모델 처리
	if (hitModelIndex >= 0)
	{
		m_gameObjects[hitModelIndex]->SetActive(false);
		if (m_objectCount > 0)
		{
			--m_objectCount;
		}

		const auto removedPolygons = static_cast<unsigned int>(
			m_gameObjects[hitModelIndex]->GetPolygonCount());
		m_polygonCount = removedPolygons <= m_polygonCount
			? m_polygonCount - removedPolygons
			: 0;
	}
}
