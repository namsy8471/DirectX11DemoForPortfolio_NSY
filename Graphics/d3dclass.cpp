////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "d3dclass.h"
#include "D3DHelpers.h"
#include <cstdlib>
#include <vector>

D3DClass::D3DClass()
{
	m_vsync_enabled = false;
	m_videoCardMemory = 0;
	// 안전한 초기화: 배열 전체를 0으로 초기화
	memset(m_videoCardDescription, 0, sizeof(m_videoCardDescription));
}


D3DClass::~D3DClass()
{
	Shutdown();
}


bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, 
						  float screenDepth, float screenNear)
{
	Shutdown();

	HRESULT result;
	Microsoft::WRL::ComPtr<IDXGIFactory> factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
	unsigned int numModes = 0, i = 0, numerator = 0, denominator = 1;
	size_t stringLength = 0;
	std::vector<DXGI_MODE_DESC> displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error = 0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	float fieldOfView, screenAspect;

	D3D11_BLEND_DESC blendStateDescription;

	// Store the vsync setting.
	m_vsync_enabled = vsync;

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(
		__uuidof(IDXGIFactory),
		reinterpret_cast<void**>(factory.ReleaseAndGetAddressOf()));
	if(FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, adapter.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, adapterOutput.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
	if(FAILED(result) || numModes == 0)
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList.resize(numModes);

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED,
		&numModes,
		displayModeList.data());
	if(FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for(i=0; i<numModes; i++)
	{
		if(displayModeList[i].Width == static_cast<unsigned int>(screenWidth)
			&& displayModeList[i].Height == static_cast<unsigned int>(screenHeight))
		{
			numerator = displayModeList[i].RefreshRate.Numerator;
			denominator = displayModeList[i].RefreshRate.Denominator;
			break;
		}
	}

	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if(FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	m_videoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	
	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, m_videoCardDescription, sizeof(m_videoCardDescription), adapterDesc.Description, sizeof(m_videoCardDescription));
	if(error != 0)
	{
		return false;
	}

	// Initialize the swap chain description.
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
    swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
    swapChainDesc.BufferDesc.Width = static_cast<UINT>(screenWidth);
    swapChainDesc.BufferDesc.Height = static_cast<UINT>(screenHeight);

	// Set regular 32-bit surface for the back buffer.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	if(m_vsync_enabled)
	{
	    swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
	    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
    swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = !fullscreen ? TRUE : FALSE;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1, 
										   D3D11_SDK_VERSION,
										   &swapChainDesc,
										   m_swapChain.ReleaseAndGetAddressOf(),
										   m_device.ReleaseAndGetAddressOf(),
										   nullptr,
										   m_deviceContext.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// Get the pointer to the back buffer.
	result = m_swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(backBuffer.ReleaseAndGetAddressOf()));
	if(FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		m_renderTargetView.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = static_cast<UINT>(screenWidth);
	depthBufferDesc.Height = static_cast<UINT>(screenHeight);
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D(
		&depthBufferDesc,
		nullptr,
		m_depthStencilBuffer.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = TRUE;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_device->CreateDepthStencilState(
		&depthStencilDesc,
		m_depthStencilState.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = m_device->CreateDepthStencilView(
		m_depthStencilBuffer.Get(),
		&depthStencilViewDesc,
		m_depthStencilView.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	ID3D11RenderTargetView* renderTargetView = m_renderTargetView.Get();
	m_deviceContext->OMSetRenderTargets(1, &renderTargetView, m_depthStencilView.Get());

	// Create rasterizer states using helper
	if (!D3DHelpers::CreateRasterizerState(
		m_device.Get(),
		D3D11_CULL_BACK,
		m_rasterState.ReleaseAndGetAddressOf()))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState.Get());
	
	// Create no-culling rasterizer state
	if (!D3DHelpers::CreateRasterizerState(
		m_device.Get(),
		D3D11_CULL_NONE,
		m_rasterStateNoCulling.ReleaseAndGetAddressOf()))
	{
		return false;
	}

	// Setup the viewport for rendering.
    m_viewport.Width = static_cast<float>(screenWidth);
    m_viewport.Height = static_cast<float>(screenHeight);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;

	// Create the viewport.
    m_deviceContext->RSSetViewports(1, &m_viewport);

	// Setup the projection matrix.
	fieldOfView = static_cast<float>(XM_PI) / 4.0f;
	screenAspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

	// Create the projection matrix for 3D rendering.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	// Initialize the world matrix to the identity matrix.
	m_worldMatrix = XMMatrixIdentity();

	// Create an orthographic projection matrix for 2D rendering.
	m_orthoMatrix = XMMatrixOrthographicLH(static_cast<float>(screenWidth), static_cast<float>(screenHeight), screenNear, screenDepth);

	// The enabled depth-stencil state was created and bound above. Only create
	// the two alternate states here; overwriting m_depthStencilState leaked the
	// first COM object in the original implementation.
	if (!D3DHelpers::CreateDepthStencilState(
		m_device.Get(),
		m_depthDisabledStencilState.ReleaseAndGetAddressOf(),
		FALSE,
		D3D11_DEPTH_WRITE_MASK_ALL))
	{
		return false;
	}
	
	// 투명 객체용 깊이 스텐실 상태 생성 (깊이 테스트 ON, 깊이 쓰기 OFF)
	if (!D3DHelpers::CreateDepthStencilState(
		m_device.Get(),
		m_depthTestNoWriteState.ReleaseAndGetAddressOf(),
		TRUE,
		D3D11_DEPTH_WRITE_MASK_ZERO))
	{
		return false;
	}

	// Clear the blend state description.
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	result = m_device->CreateBlendState(
		&blendStateDescription,
		m_alphaEnableBlendingState.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	if (!D3DHelpers::CreateBlendState(
		m_device.Get(),
		m_alphaDisableBlendingState.ReleaseAndGetAddressOf(),
		FALSE))
	{
		return false;
	}
	
    return true;
}


void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if(m_swapChain)
	{
		m_swapChain->SetFullscreenState(FALSE, nullptr);
	}

	if(m_deviceContext)
	{
		m_deviceContext->ClearState();
	}

	m_alphaDisableBlendingState.Reset();
	m_alphaEnableBlendingState.Reset();
	m_depthTestNoWriteState.Reset();
	m_depthDisabledStencilState.Reset();
	m_rasterStateNoCulling.Reset();
	m_rasterState.Reset();
	m_depthStencilView.Reset();
	m_depthStencilState.Reset();
	m_depthStencilBuffer.Reset();
	m_renderTargetView.Reset();
	m_deviceContext.Reset();
	m_swapChain.Reset();
	m_device.Reset();
}


void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);
    
	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


bool D3DClass::EndScene() noexcept
{
	if (!m_swapChain)
	{
		return false;
	}

	const UINT syncInterval = m_vsync_enabled ? 1u : 0u;
	return SUCCEEDED(m_swapChain->Present(syncInterval, 0u));
}


ID3D11Device* D3DClass::GetDevice()
{
	return m_device.Get();
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext.Get();
}


void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}


void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}

void D3DClass::TurnZBufferOn()
{
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
	return;
}


void D3DClass::TurnZBufferOff()
{
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState.Get(), 1);
	return;
}


// 공통 블렌딩 상태 설정 함수로 중복 제거
void D3DClass::SetBlendState(ID3D11BlendState* state)
{
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_deviceContext->OMSetBlendState(state, blendFactor, 0xffffffff);
}

void D3DClass::TurnOnAlphaBlending()
{
	SetBlendState(m_alphaEnableBlendingState.Get());
}

void D3DClass::TurnOffAlphaBlending()
{
	SetBlendState(m_alphaDisableBlendingState.Get());
}

void D3DClass::SetBackBufferRenderTarget()
{
	// 렌더 타겟 뷰와 깊이 스텐실 버퍼를 출력 렌더 파이프 라인에 바인딩합니다.
	ID3D11RenderTargetView* renderTargetView = m_renderTargetView.Get();
	m_deviceContext->OMSetRenderTargets(1, &renderTargetView, m_depthStencilView.Get());
}


void D3DClass::ResetViewport()
{
	// 뷰포트를 재설정합니다.
	m_deviceContext->RSSetViewports(1, &m_viewport);
}


void D3DClass::TurnOnCulling()
{
	// 컬링 래스터 라이저 상태를 설정합니다.
	m_deviceContext->RSSetState(m_rasterState.Get());
}


void D3DClass::TurnOffCulling()
{
	// 뒷면 없음 컬링 래스터 라이저 상태를 설정합니다.
	m_deviceContext->RSSetState(m_rasterStateNoCulling.Get());
}

// 투명 객체용 깊이 스텐실 상태 활성화 (깊이 테스트 ON, 깊이 쓰기 OFF)
void D3DClass::EnableDepthTestingWithoutWrites()
{
	m_deviceContext->OMSetDepthStencilState(m_depthTestNoWriteState.Get(), 1);
}

// 일반 깊이 스텐실 상태로 복원 (깊이 테스트 ON, 깊이 쓰기 ON)
void D3DClass::DisableDepthTestingWithoutWrites()
{
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
}
