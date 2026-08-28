#include "Engine/Rendering/Textures/TextureResources.h"

#include "Engine/Rendering/Textures/DDSTextureLoader.h"

namespace Engine::Rendering
{
	bool DdsTexture::Initialize(ID3D11Device* device, const wchar_t* filename) noexcept
	{
		m_shaderResourceView.Reset();
		if (device == nullptr || filename == nullptr || *filename == L'\0')
		{
			return false;
		}

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
		if (FAILED(DirectX::CreateDDSTextureFromFile(
			device,
			filename,
			nullptr,
			shaderResourceView.GetAddressOf())))
		{
			return false;
		}

		m_shaderResourceView = std::move(shaderResourceView);
		return true;
	}

	ID3D11ShaderResourceView* DdsTexture::GetShaderResourceView() const noexcept
	{
		return m_shaderResourceView.Get();
	}

	bool RenderTargetTexture::Initialize(
		ID3D11Device* device,
		const RenderTextureDescriptor& descriptor) noexcept
	{
		Reset();
		if (device == nullptr || descriptor.width <= 0 || descriptor.height <= 0 ||
			descriptor.screenNear <= 0.0f || descriptor.screenDepth <= descriptor.screenNear)
		{
			return false;
		}

		D3D11_TEXTURE2D_DESC textureDescription{};
		textureDescription.Width = static_cast<UINT>(descriptor.width);
		textureDescription.Height = static_cast<UINT>(descriptor.height);
		textureDescription.MipLevels = 1;
		textureDescription.ArraySize = 1;
		textureDescription.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDescription.SampleDesc.Count = 1;
		textureDescription.Usage = D3D11_USAGE_DEFAULT;
		textureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetTexture;
		if (FAILED(device->CreateTexture2D(
			&textureDescription,
			nullptr,
			renderTargetTexture.GetAddressOf())))
		{
			return false;
		}

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDescription{};
		renderTargetViewDescription.Format = textureDescription.Format;
		renderTargetViewDescription.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
		if (FAILED(device->CreateRenderTargetView(
			renderTargetTexture.Get(),
			&renderTargetViewDescription,
			renderTargetView.GetAddressOf())))
		{
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription{};
		shaderResourceViewDescription.Format = textureDescription.Format;
		shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDescription.Texture2D.MipLevels = 1;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
		if (FAILED(device->CreateShaderResourceView(
			renderTargetTexture.Get(),
			&shaderResourceViewDescription,
			shaderResourceView.GetAddressOf())))
		{
			return false;
		}

		D3D11_TEXTURE2D_DESC depthBufferDescription{};
		depthBufferDescription.Width = static_cast<UINT>(descriptor.width);
		depthBufferDescription.Height = static_cast<UINT>(descriptor.height);
		depthBufferDescription.MipLevels = 1;
		depthBufferDescription.ArraySize = 1;
		depthBufferDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDescription.SampleDesc.Count = 1;
		depthBufferDescription.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
		if (FAILED(device->CreateTexture2D(
			&depthBufferDescription,
			nullptr,
			depthStencilBuffer.GetAddressOf())))
		{
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription{};
		depthStencilViewDescription.Format = depthBufferDescription.Format;
		depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
		if (FAILED(device->CreateDepthStencilView(
			depthStencilBuffer.Get(),
			&depthStencilViewDescription,
			depthStencilView.GetAddressOf())))
		{
			return false;
		}

		m_viewport.Width = static_cast<float>(descriptor.width);
		m_viewport.Height = static_cast<float>(descriptor.height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XM_PI / 4.0f,
			static_cast<float>(descriptor.width) / static_cast<float>(descriptor.height),
			descriptor.screenNear,
			descriptor.screenDepth);
		m_orthoMatrix = DirectX::XMMatrixOrthographicLH(
			static_cast<float>(descriptor.width),
			static_cast<float>(descriptor.height),
			descriptor.screenNear,
			descriptor.screenDepth);

		m_renderTargetTexture = std::move(renderTargetTexture);
		m_renderTargetView = std::move(renderTargetView);
		m_shaderResourceView = std::move(shaderResourceView);
		m_depthStencilBuffer = std::move(depthStencilBuffer);
		m_depthStencilView = std::move(depthStencilView);
		return true;
	}

	void RenderTargetTexture::SetRenderTarget(ID3D11DeviceContext* deviceContext) noexcept
	{
		if (deviceContext == nullptr || !m_renderTargetView || !m_depthStencilView)
		{
			return;
		}

		ID3D11RenderTargetView* renderTargetView = m_renderTargetView.Get();
		deviceContext->OMSetRenderTargets(1, &renderTargetView, m_depthStencilView.Get());
		deviceContext->RSSetViewports(1, &m_viewport);
	}

	void RenderTargetTexture::ClearRenderTarget(
		ID3D11DeviceContext* deviceContext,
		float red,
		float green,
		float blue,
		float alpha) noexcept
	{
		if (deviceContext == nullptr || !m_renderTargetView || !m_depthStencilView)
		{
			return;
		}

		const float color[4] = { red, green, blue, alpha };
		deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);
		deviceContext->ClearDepthStencilView(
			m_depthStencilView.Get(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0);
	}

	ID3D11ShaderResourceView* RenderTargetTexture::GetShaderResourceView() const noexcept
	{
		return m_shaderResourceView.Get();
	}

	void RenderTargetTexture::GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix) const noexcept
	{
		projectionMatrix = m_projectionMatrix;
	}

	void RenderTargetTexture::GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix) const noexcept
	{
		orthoMatrix = m_orthoMatrix;
	}

	void RenderTargetTexture::Reset() noexcept
	{
		m_depthStencilView.Reset();
		m_depthStencilBuffer.Reset();
		m_shaderResourceView.Reset();
		m_renderTargetView.Reset();
		m_renderTargetTexture.Reset();
		m_viewport = {};
		m_projectionMatrix = DirectX::XMMatrixIdentity();
		m_orthoMatrix = DirectX::XMMatrixIdentity();
	}
}
