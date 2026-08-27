#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "AlignedAllocationPolicy.h"

using namespace DirectX;

class RenderTextureClass : public AlignedAllocationPolicy<16>
{
public:
	RenderTextureClass();
	RenderTextureClass(const RenderTextureClass&) = delete;
	RenderTextureClass& operator=(const RenderTextureClass&) = delete;
	~RenderTextureClass();

	bool Initialize(ID3D11Device*, int, int, float, float);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext*);
	void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);
	ID3D11ShaderResourceView* GetShaderResourceView();

	void GetProjectionMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

private:
	int m_textureWidth = 0;
	int m_textureHeight = 0;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	D3D11_VIEWPORT m_viewport{};
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_orthoMatrix;
};
