#include "stdafx.h"
#include "RenderTextureClass.h"


RenderTextureClass::RenderTextureClass()
{
}


RenderTextureClass::~RenderTextureClass()
{
	Shutdown();
}


bool RenderTextureClass::Initialize(ID3D11Device* device, int textureWidth, int textureHeight, float screenDepth, float screenNear)
{
	Shutdown();
	if (device == nullptr || textureWidth <= 0 || textureHeight <= 0)
	{
		return false;
	}

	// ������ �ؽ�ó�� ���� ���̸� �����մϴ�.
	m_textureWidth = textureWidth;
	m_textureHeight = textureHeight;

	// ���� Ÿ�� �ؽ�ó ������ �ʱ�ȭ�մϴ�.
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// ���� Ÿ�� �ؽ�ó ������ �����մϴ�.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// ���� Ÿ�� �ؽ�ó�� ����ϴ�.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetTexture;
	HRESULT result = device->CreateTexture2D(&textureDesc, nullptr, renderTargetTexture.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// ���� Ÿ�� ���� ������ �����մϴ�.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// ���� Ÿ�� �並 �����Ѵ�.
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	result = device->CreateRenderTargetView(renderTargetTexture.Get(), &renderTargetViewDesc, renderTargetView.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// ���̴� ���ҽ� ���� ������ �����մϴ�.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// ���̴� ���ҽ� �並 ����ϴ�.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	result = device->CreateShaderResourceView(renderTargetTexture.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// ���� ������ ����ü�� �ʱ�ȭ �մϴ�.
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// ���� ���� ����ü�� �����մϴ�.
	depthBufferDesc.Width = textureWidth;
	depthBufferDesc.Height = textureHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// ä���� ����ü�� ����Ͽ� ���� ������ �ؽ�ó�� ����ϴ�.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	result = device->CreateTexture2D(&depthBufferDesc, nullptr, depthStencilBuffer.GetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// ���� ���ٽ� �並 �ʱ�ȭ�մϴ�.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// ���� ���ٽ� �� ����ü�� �����մϴ�.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// ���� ���ٽ� �並 ����ϴ�.
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	result = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// �������� ���� ����Ʈ�� �����մϴ�.
    m_viewport.Width = (float)textureWidth;
    m_viewport.Height = (float)textureHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;

	// ���� ����� �����մϴ�.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(((float)XM_PI / 4.0f), ((float)textureWidth / (float)textureHeight), screenNear, screenDepth);

	// 2D �������� ���� ���� ���� ����� ����ϴ�.
	m_orthoMatrix = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, screenNear, screenDepth);

	m_renderTargetTexture.Swap(renderTargetTexture);
	m_renderTargetView.Swap(renderTargetView);
	m_shaderResourceView.Swap(shaderResourceView);
	m_depthStencilBuffer.Swap(depthStencilBuffer);
	m_depthStencilView.Swap(depthStencilView);

	return true;
}


void RenderTextureClass::Shutdown()
{
	m_depthStencilView.Reset();
	m_depthStencilBuffer.Reset();
	m_shaderResourceView.Reset();
	m_renderTargetView.Reset();
	m_renderTargetTexture.Reset();
	m_textureWidth = 0;
	m_textureHeight = 0;
}


void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext* deviceContext)
{
	// ������ ��� ��� ���� ���ٽ� ���۸� ��� ���� ������ ���ο� ���ε��մϴ�.
	ID3D11RenderTargetView* renderTargetView = m_renderTargetView.Get();
	deviceContext->OMSetRenderTargets(1, &renderTargetView, m_depthStencilView.Get());
	
	// ����Ʈ�� �����մϴ�.
    deviceContext->RSSetViewports(1, &m_viewport);
}


void RenderTextureClass::ClearRenderTarget(ID3D11DeviceContext* deviceContext, float red, float green, float blue, float alpha)
{
	// ���۸� ���� ���� �����մϴ�.
	float color[4] = { red, green, blue, alpha };

	// �� ���۸� �����.
	deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);

	// ���� ���۸� �����.
	deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}


ID3D11ShaderResourceView* RenderTextureClass::GetShaderResourceView()
{
	return m_shaderResourceView.Get();
}


void RenderTextureClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}


void RenderTextureClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
}
