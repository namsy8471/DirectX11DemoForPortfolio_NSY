#pragma once

#include "Engine/Rendering/Textures/ITexture.h"

#include <wrl/client.h>

namespace Engine::Rendering
{
	struct RenderTextureDescriptor final
	{
		int width = 0;
		int height = 0;
		float screenDepth = 1000.0f;
		float screenNear = 0.1f;
	};

	class DdsTexture final : public ITexture
	{
	public:
		DdsTexture() = default;
		~DdsTexture() override = default;

		DdsTexture(const DdsTexture&) = delete;
		DdsTexture& operator=(const DdsTexture&) = delete;
		DdsTexture(DdsTexture&&) noexcept = default;
		DdsTexture& operator=(DdsTexture&&) noexcept = default;

		[[nodiscard]] bool Initialize(ID3D11Device* device, const wchar_t* filename) noexcept;
		[[nodiscard]] ID3D11ShaderResourceView* GetShaderResourceView() const noexcept override;

	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	};

	class alignas(16) RenderTargetTexture final : public IRenderTexture
	{
	public:
		RenderTargetTexture() = default;
		~RenderTargetTexture() override = default;

		RenderTargetTexture(const RenderTargetTexture&) = delete;
		RenderTargetTexture& operator=(const RenderTargetTexture&) = delete;
		RenderTargetTexture(RenderTargetTexture&&) noexcept = default;
		RenderTargetTexture& operator=(RenderTargetTexture&&) noexcept = default;

		[[nodiscard]] bool Initialize(
			ID3D11Device* device,
			const RenderTextureDescriptor& descriptor) noexcept;

		void SetRenderTarget(ID3D11DeviceContext* deviceContext) noexcept override;
		void ClearRenderTarget(
			ID3D11DeviceContext* deviceContext,
			float red,
			float green,
			float blue,
			float alpha) noexcept override;

		[[nodiscard]] ID3D11ShaderResourceView* GetShaderResourceView() const noexcept override;
		void GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix) const noexcept override;
		void GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix) const noexcept override;

	private:
		void Reset() noexcept;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTargetTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
		D3D11_VIEWPORT m_viewport{};
		DirectX::XMMATRIX m_projectionMatrix = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX m_orthoMatrix = DirectX::XMMatrixIdentity();
	};
}
