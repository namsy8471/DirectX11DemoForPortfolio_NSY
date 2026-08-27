#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include <memory>
#include <vector>

class CameraClass;
class D3DClass;
class DepthShaderClass;
class HorizontalBlurShaderClass;
class LightClass;
class OrthoWindowClass;
class RenderTextureClass;
class ShadowShaderClass;
class SoftShadowShaderClass;
class TerrainClass;
class TextureShaderClass;
class VerticalBlurShaderClass;

namespace Engine::Scene
{
	class GameObject;
	struct SceneDefinition;
}

namespace Engine::Rendering
{
	struct SoftShadowPipelineConfig final
	{
		int shadowMapWidth = 0;
		int shadowMapHeight = 0;
		float screenDepth = 1000.0f;
		float screenNear = 0.1f;
	};

	struct SoftShadowPipelineFrame final
	{
		D3DClass& d3d;
		CameraClass& camera;
		LightClass& light;
		TerrainClass& terrain;
		const std::vector<std::unique_ptr<Scene::GameObject>>& gameObjects;
		TextureShaderClass& textureShader;
		const Scene::SceneDefinition& sceneDefinition;
	};

	class SoftShadowPipeline final
	{
	public:
		SoftShadowPipeline();
		~SoftShadowPipeline();

		SoftShadowPipeline(const SoftShadowPipeline&) = delete;
		SoftShadowPipeline& operator=(const SoftShadowPipeline&) = delete;
		SoftShadowPipeline(SoftShadowPipeline&&) = delete;
		SoftShadowPipeline& operator=(SoftShadowPipeline&&) = delete;

		bool Initialize(
			ID3D11Device* device,
			HWND window,
			const SoftShadowPipelineConfig& config);
		void Shutdown() noexcept;

		bool Execute(const SoftShadowPipelineFrame& frame);
		bool RenderSurface(
			ID3D11DeviceContext* deviceContext,
			int indexCount,
			DirectX::XMMATRIX worldMatrix,
			DirectX::XMMATRIX viewMatrix,
			DirectX::XMMATRIX projectionMatrix,
			ID3D11ShaderResourceView* texture,
			LightClass& light);

		[[nodiscard]] ID3D11ShaderResourceView* GetBlurredShadowShaderResourceView() const noexcept;
		[[nodiscard]] bool IsInitialized() const noexcept;

	private:
		bool RenderSceneToTexture(const SoftShadowPipelineFrame& frame);
		bool RenderBlackAndWhiteShadows(const SoftShadowPipelineFrame& frame);
		bool DownSampleTexture(const SoftShadowPipelineFrame& frame);
		bool RenderHorizontalBlurToTexture(const SoftShadowPipelineFrame& frame);
		bool RenderVerticalBlurToTexture(const SoftShadowPipelineFrame& frame);
		bool UpSampleTexture(const SoftShadowPipelineFrame& frame);

		SoftShadowPipelineConfig m_config;
		bool m_initialized = false;

		std::unique_ptr<RenderTextureClass> m_shadowTexture;
		std::unique_ptr<RenderTextureClass> m_blackWhiteShadowTexture;
		std::unique_ptr<RenderTextureClass> m_downsampleTexture;
		std::unique_ptr<RenderTextureClass> m_horizontalBlurTexture;
		std::unique_ptr<RenderTextureClass> m_verticalBlurTexture;
		std::unique_ptr<RenderTextureClass> m_upsampleTexture;

		std::unique_ptr<DepthShaderClass> m_depthShader;
		std::unique_ptr<ShadowShaderClass> m_shadowShader;
		std::unique_ptr<HorizontalBlurShaderClass> m_horizontalBlurShader;
		std::unique_ptr<VerticalBlurShaderClass> m_verticalBlurShader;
		std::unique_ptr<SoftShadowShaderClass> m_softShadowShader;

		std::unique_ptr<OrthoWindowClass> m_smallWindow;
		std::unique_ptr<OrthoWindowClass> m_fullScreenWindow;
	};
}
