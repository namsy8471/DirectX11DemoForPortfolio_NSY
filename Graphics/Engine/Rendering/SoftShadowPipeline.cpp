#include "SoftShadowPipeline.h"

#include "../Core/ScopeExit.h"
#include "../Scene/GameObject.h"
#include "../Scene/SceneDefinition.h"

#include "../../cameraclass.h"
#include "../../d3dclass.h"
#include "../../DepthShaderClass.h"
#include "../../HorizontalBlurShaderClass.h"
#include "../../LightClass.h"
#include "../../OrthoWindowClass.h"
#include "../../RenderTextureClass.h"
#include "../../ShadowShaderClass.h"
#include "../../SoftShadowShaderClass.h"
#include "../../TerrainClass.h"
#include "../../textureshaderclass.h"
#include "../../VerticalBlurShaderClass.h"

#include <utility>

namespace Engine::Rendering
{
	SoftShadowPipeline::SoftShadowPipeline() = default;

	SoftShadowPipeline::~SoftShadowPipeline()
	{
		Shutdown();
	}

	bool SoftShadowPipeline::Initialize(
		ID3D11Device* device,
		HWND window,
		const SoftShadowPipelineConfig& config)
	{
		Shutdown();

		if (device == nullptr || config.shadowMapWidth <= 0 || config.shadowMapHeight <= 0 ||
			config.screenDepth <= config.screenNear || config.screenNear <= 0.0f)
		{
			return false;
		}

		m_config = config;
		const int downSampleWidth = config.shadowMapWidth / 2;
		const int downSampleHeight = config.shadowMapHeight / 2;
		if (downSampleWidth <= 0 || downSampleHeight <= 0)
		{
			Shutdown();
			return false;
		}

		auto fail = [this, window](const wchar_t* message)
		{
			MessageBox(window, message, L"Error", MB_OK);
			Shutdown();
			return false;
		};

		m_shadowTexture = std::make_unique<RenderTextureClass>();
		if (!m_shadowTexture->Initialize(
			device,
			config.shadowMapWidth,
			config.shadowMapHeight,
			config.screenDepth,
			config.screenNear))
		{
			return fail(L"Could not initialize the render to texture object.");
		}

		m_depthShader = std::make_unique<DepthShaderClass>();
		if (!m_depthShader->Initialize(device, window))
		{
			return fail(L"Could not initialize the depth shader object.");
		}

		m_blackWhiteShadowTexture = std::make_unique<RenderTextureClass>();
		if (!m_blackWhiteShadowTexture->Initialize(
			device,
			config.shadowMapWidth,
			config.shadowMapHeight,
			config.screenDepth,
			config.screenNear))
		{
			return fail(L"Could not initialize the black and white render to texture object.");
		}

		m_shadowShader = std::make_unique<ShadowShaderClass>();
		if (!m_shadowShader->Initialize(device, window))
		{
			return fail(L"Could not initialize the shadow shader object.");
		}

		m_downsampleTexture = std::make_unique<RenderTextureClass>();
		if (!m_downsampleTexture->Initialize(device, downSampleWidth, downSampleHeight, 100.0f, 1.0f))
		{
			return fail(L"Could not initialize the down sample render to texture object.");
		}

		m_smallWindow = std::make_unique<OrthoWindowClass>();
		if (!m_smallWindow->Initialize(device, downSampleWidth, downSampleHeight))
		{
			return fail(L"Could not initialize the small ortho window object.");
		}

		m_horizontalBlurTexture = std::make_unique<RenderTextureClass>();
		if (!m_horizontalBlurTexture->Initialize(
			device,
			downSampleWidth,
			downSampleHeight,
			config.screenDepth,
			0.1f))
		{
			return fail(L"Could not initialize the horizontal blur render to texture object.");
		}

		m_horizontalBlurShader = std::make_unique<HorizontalBlurShaderClass>();
		if (!m_horizontalBlurShader->Initialize(device, window))
		{
			return fail(L"Could not initialize the horizontal blur shader object.");
		}

		m_verticalBlurTexture = std::make_unique<RenderTextureClass>();
		if (!m_verticalBlurTexture->Initialize(
			device,
			downSampleWidth,
			downSampleHeight,
			config.screenDepth,
			0.1f))
		{
			return fail(L"Could not initialize the vertical blur render to texture object.");
		}

		m_verticalBlurShader = std::make_unique<VerticalBlurShaderClass>();
		if (!m_verticalBlurShader->Initialize(device, window))
		{
			return fail(L"Could not initialize the vertical blur shader object.");
		}

		m_upsampleTexture = std::make_unique<RenderTextureClass>();
		if (!m_upsampleTexture->Initialize(
			device,
			config.shadowMapWidth,
			config.shadowMapHeight,
			config.screenDepth,
			0.1f))
		{
			return fail(L"Could not initialize the up sample render to texture object.");
		}

		m_fullScreenWindow = std::make_unique<OrthoWindowClass>();
		if (!m_fullScreenWindow->Initialize(device, config.shadowMapWidth, config.shadowMapHeight))
		{
			return fail(L"Could not initialize the full screen ortho window object.");
		}

		m_softShadowShader = std::make_unique<SoftShadowShaderClass>();
		if (!m_softShadowShader->Initialize(device, window))
		{
			return fail(L"Could not initialize the soft shadow shader object.");
		}

		m_initialized = true;
		return true;
	}

	void SoftShadowPipeline::Shutdown() noexcept
	{
		m_initialized = false;

		m_softShadowShader.reset();
		m_fullScreenWindow.reset();
		m_upsampleTexture.reset();
		m_verticalBlurShader.reset();
		m_verticalBlurTexture.reset();
		m_horizontalBlurShader.reset();
		m_horizontalBlurTexture.reset();
		m_smallWindow.reset();
		m_downsampleTexture.reset();
		m_shadowShader.reset();
		m_blackWhiteShadowTexture.reset();
		m_depthShader.reset();
		m_shadowTexture.reset();

		m_config = {};
	}

	bool SoftShadowPipeline::Execute(const SoftShadowPipelineFrame& frame)
	{
		if (!m_initialized)
		{
			return false;
		}

		return RenderSceneToTexture(frame) &&
			RenderBlackAndWhiteShadows(frame) &&
			DownSampleTexture(frame) &&
			RenderHorizontalBlurToTexture(frame) &&
			RenderVerticalBlurToTexture(frame) &&
			UpSampleTexture(frame);
	}

	bool SoftShadowPipeline::RenderSurface(
		ID3D11DeviceContext* deviceContext,
		int indexCount,
		DirectX::XMMATRIX worldMatrix,
		DirectX::XMMATRIX viewMatrix,
		DirectX::XMMATRIX projectionMatrix,
		ID3D11ShaderResourceView* texture,
		LightClass& light)
	{
		if (!m_initialized || deviceContext == nullptr || m_softShadowShader == nullptr)
		{
			return false;
		}

		return m_softShadowShader->Render(
			deviceContext,
			indexCount,
			worldMatrix,
			viewMatrix,
			projectionMatrix,
			texture,
			GetBlurredShadowShaderResourceView(),
			light.GetDirection(),
			light.GetAmbientColor(),
			light.GetDiffuseColor(),
			light.GetPosition(),
			light.GetSpecularColor(),
			light.GetSpecularPower());
	}

	ID3D11ShaderResourceView* SoftShadowPipeline::GetBlurredShadowShaderResourceView() const noexcept
	{
		return m_upsampleTexture != nullptr
			? m_upsampleTexture->GetShaderResourceView()
			: nullptr;
	}

	bool SoftShadowPipeline::IsInitialized() const noexcept
	{
		return m_initialized;
	}

	bool SoftShadowPipeline::RenderSceneToTexture(const SoftShadowPipelineFrame& frame)
	{
		[[maybe_unused]] auto restorePipeline = MakeScopeExit([&frame]() noexcept {
			frame.d3d.SetBackBufferRenderTarget();
			frame.d3d.ResetViewport();
			frame.d3d.TurnZBufferOn();
		});

		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX lightViewMatrix;
		DirectX::XMMATRIX lightProjectionMatrix;

		m_shadowTexture->SetRenderTarget(frame.d3d.GetDeviceContext());
		m_shadowTexture->ClearRenderTarget(frame.d3d.GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

		frame.light.GenerateViewMatrix();
		frame.d3d.GetWorldMatrix(worldMatrix);
		frame.light.GetViewMatrix(lightViewMatrix);
		frame.light.GetProjectionMatrix(lightProjectionMatrix);

		frame.d3d.GetWorldMatrix(worldMatrix);
		const auto& terrainOffset = frame.sceneDefinition.terrain.worldOffset;
		worldMatrix *= DirectX::XMMatrixTranslation(terrainOffset.x, terrainOffset.y, terrainOffset.z);

		frame.terrain.Render(frame.d3d.GetDeviceContext());
		if (!m_depthShader->Render(
			frame.d3d.GetDeviceContext(),
			frame.terrain.GetIndexCount(),
			worldMatrix,
			lightViewMatrix,
			lightProjectionMatrix))
		{
			return false;
		}

		for (const auto& gameObject : frame.gameObjects)
		{
			if (!gameObject->IsActive())
			{
				continue;
			}

			worldMatrix = gameObject->GetWorldMatrix();
			gameObject->Render(frame.d3d.GetDeviceContext());
			if (!m_depthShader->Render(
				frame.d3d.GetDeviceContext(),
				gameObject->GetIndexCount(),
				worldMatrix,
				lightViewMatrix,
				lightProjectionMatrix))
			{
				return false;
			}
		}

		frame.d3d.SetBackBufferRenderTarget();
		frame.d3d.ResetViewport();
		return true;
	}

	bool SoftShadowPipeline::RenderBlackAndWhiteShadows(const SoftShadowPipelineFrame& frame)
	{
		[[maybe_unused]] auto restorePipeline = MakeScopeExit([&frame]() noexcept {
			frame.d3d.SetBackBufferRenderTarget();
			frame.d3d.ResetViewport();
			frame.d3d.TurnZBufferOn();
		});

		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
		DirectX::XMMATRIX lightViewMatrix;
		DirectX::XMMATRIX lightProjectionMatrix;
		DirectX::XMMATRIX terrainMatrix;

		m_blackWhiteShadowTexture->SetRenderTarget(frame.d3d.GetDeviceContext());
		m_blackWhiteShadowTexture->ClearRenderTarget(frame.d3d.GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

		frame.camera.UpdateViewMatrix();
		frame.light.GenerateViewMatrix();
		frame.camera.GetViewMatrix(viewMatrix);
		frame.d3d.GetWorldMatrix(worldMatrix);
		frame.d3d.GetProjectionMatrix(projectionMatrix);
		frame.light.GetViewMatrix(lightViewMatrix);
		frame.light.GetProjectionMatrix(lightProjectionMatrix);

		frame.d3d.GetWorldMatrix(terrainMatrix);
		const auto& terrainOffset = frame.sceneDefinition.terrain.worldOffset;
		terrainMatrix *= DirectX::XMMatrixTranslation(terrainOffset.x, terrainOffset.y, terrainOffset.z);

		frame.terrain.Render(frame.d3d.GetDeviceContext());
		if (!m_shadowShader->Render(
			frame.d3d.GetDeviceContext(),
			frame.terrain.GetIndexCount(),
			terrainMatrix,
			viewMatrix,
			projectionMatrix,
			lightViewMatrix,
			lightProjectionMatrix,
			m_shadowTexture->GetShaderResourceView(),
			frame.light.GetPosition()))
		{
			return false;
		}

		for (const auto& gameObject : frame.gameObjects)
		{
			if (!gameObject->IsActive())
			{
				continue;
			}

			worldMatrix = gameObject->GetWorldMatrix();
			gameObject->Render(frame.d3d.GetDeviceContext());
			if (!m_shadowShader->Render(
				frame.d3d.GetDeviceContext(),
				gameObject->GetIndexCount(),
				worldMatrix,
				viewMatrix,
				projectionMatrix,
				lightViewMatrix,
				lightProjectionMatrix,
				m_shadowTexture->GetShaderResourceView(),
				frame.light.GetPosition()))
			{
				return false;
			}
		}

		frame.d3d.SetBackBufferRenderTarget();
		frame.d3d.ResetViewport();
		return true;
	}

	bool SoftShadowPipeline::DownSampleTexture(const SoftShadowPipelineFrame& frame)
	{
		[[maybe_unused]] auto restorePipeline = MakeScopeExit([&frame]() noexcept {
			frame.d3d.SetBackBufferRenderTarget();
			frame.d3d.ResetViewport();
			frame.d3d.TurnZBufferOn();
		});

		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX baseViewMatrix;
		DirectX::XMMATRIX orthoMatrix;

		m_downsampleTexture->SetRenderTarget(frame.d3d.GetDeviceContext());
		m_downsampleTexture->ClearRenderTarget(frame.d3d.GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

		frame.camera.UpdateViewMatrix();
		frame.d3d.GetWorldMatrix(worldMatrix);
		frame.camera.GetBaseViewMatrix(baseViewMatrix);
		m_downsampleTexture->GetOrthoMatrix(orthoMatrix);

		frame.d3d.TurnZBufferOff();
		m_smallWindow->Render(frame.d3d.GetDeviceContext());
		if (!frame.textureShader.Render(
			frame.d3d.GetDeviceContext(),
			m_smallWindow->GetIndexCount(),
			worldMatrix,
			baseViewMatrix,
			orthoMatrix,
			m_blackWhiteShadowTexture->GetShaderResourceView()))
		{
			return false;
		}

		frame.d3d.TurnZBufferOn();
		frame.d3d.SetBackBufferRenderTarget();
		frame.d3d.ResetViewport();
		return true;
	}

	bool SoftShadowPipeline::RenderHorizontalBlurToTexture(const SoftShadowPipelineFrame& frame)
	{
		[[maybe_unused]] auto restorePipeline = MakeScopeExit([&frame]() noexcept {
			frame.d3d.SetBackBufferRenderTarget();
			frame.d3d.ResetViewport();
			frame.d3d.TurnZBufferOn();
		});

		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX baseViewMatrix;
		DirectX::XMMATRIX orthoMatrix;
		const float screenSizeX = static_cast<float>(m_config.shadowMapWidth) / 2.0f;

		m_horizontalBlurTexture->SetRenderTarget(frame.d3d.GetDeviceContext());
		m_horizontalBlurTexture->ClearRenderTarget(frame.d3d.GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

		frame.camera.UpdateViewMatrix();
		frame.camera.GetBaseViewMatrix(baseViewMatrix);
		frame.d3d.GetWorldMatrix(worldMatrix);
		m_horizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

		frame.d3d.TurnZBufferOff();
		m_smallWindow->Render(frame.d3d.GetDeviceContext());
		if (!m_horizontalBlurShader->Render(
			frame.d3d.GetDeviceContext(),
			m_smallWindow->GetIndexCount(),
			worldMatrix,
			baseViewMatrix,
			orthoMatrix,
			m_downsampleTexture->GetShaderResourceView(),
			screenSizeX))
		{
			return false;
		}

		frame.d3d.TurnZBufferOn();
		frame.d3d.SetBackBufferRenderTarget();
		frame.d3d.ResetViewport();
		return true;
	}

	bool SoftShadowPipeline::RenderVerticalBlurToTexture(const SoftShadowPipelineFrame& frame)
	{
		[[maybe_unused]] auto restorePipeline = MakeScopeExit([&frame]() noexcept {
			frame.d3d.SetBackBufferRenderTarget();
			frame.d3d.ResetViewport();
			frame.d3d.TurnZBufferOn();
		});

		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX baseViewMatrix;
		DirectX::XMMATRIX orthoMatrix;
		const float screenSizeY = static_cast<float>(m_config.shadowMapHeight) / 2.0f;

		m_verticalBlurTexture->SetRenderTarget(frame.d3d.GetDeviceContext());
		m_verticalBlurTexture->ClearRenderTarget(frame.d3d.GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

		frame.camera.UpdateViewMatrix();
		frame.camera.GetBaseViewMatrix(baseViewMatrix);
		frame.d3d.GetWorldMatrix(worldMatrix);
		m_verticalBlurTexture->GetOrthoMatrix(orthoMatrix);

		frame.d3d.TurnZBufferOff();
		m_smallWindow->Render(frame.d3d.GetDeviceContext());
		if (!m_verticalBlurShader->Render(
			frame.d3d.GetDeviceContext(),
			m_smallWindow->GetIndexCount(),
			worldMatrix,
			baseViewMatrix,
			orthoMatrix,
			m_horizontalBlurTexture->GetShaderResourceView(),
			screenSizeY))
		{
			return false;
		}

		frame.d3d.TurnZBufferOn();
		frame.d3d.SetBackBufferRenderTarget();
		frame.d3d.ResetViewport();
		return true;
	}

	bool SoftShadowPipeline::UpSampleTexture(const SoftShadowPipelineFrame& frame)
	{
		[[maybe_unused]] auto restorePipeline = MakeScopeExit([&frame]() noexcept {
			frame.d3d.SetBackBufferRenderTarget();
			frame.d3d.ResetViewport();
			frame.d3d.TurnZBufferOn();
		});

		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX baseViewMatrix;
		DirectX::XMMATRIX orthoMatrix;

		m_upsampleTexture->SetRenderTarget(frame.d3d.GetDeviceContext());
		m_upsampleTexture->ClearRenderTarget(frame.d3d.GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

		frame.camera.UpdateViewMatrix();
		frame.camera.GetBaseViewMatrix(baseViewMatrix);
		frame.d3d.GetWorldMatrix(worldMatrix);
		m_upsampleTexture->GetOrthoMatrix(orthoMatrix);

		frame.d3d.TurnZBufferOff();
		m_fullScreenWindow->Render(frame.d3d.GetDeviceContext());
		if (!frame.textureShader.Render(
			frame.d3d.GetDeviceContext(),
			m_fullScreenWindow->GetIndexCount(),
			worldMatrix,
			baseViewMatrix,
			orthoMatrix,
			m_verticalBlurTexture->GetShaderResourceView()))
		{
			return false;
		}

		frame.d3d.TurnZBufferOn();
		frame.d3d.SetBackBufferRenderTarget();
		frame.d3d.ResetViewport();
		return true;
	}
}
