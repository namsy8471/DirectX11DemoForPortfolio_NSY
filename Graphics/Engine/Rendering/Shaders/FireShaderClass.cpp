#include "stdafx.h"
#include "Engine/Rendering/Shaders/FireShaderClass.h"


FireShaderClass::FireShaderClass()
{
}


FireShaderClass::~FireShaderClass()
{
	Shutdown();
}


bool FireShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	// 정점 및 픽셀 쉐이더를 초기화합니다.
	ShutdownShader();
	if (!InitializeShader(device, hwnd, L"./data/fire_vs.hlsl", L"./data/fire_ps.hlsl"))
	{
		ShutdownShader();
		return false;
	}

	return true;
}


void FireShaderClass::Shutdown()
{
	// 버텍스 및 픽셀 쉐이더와 관련된 객체를 종료합니다.
	ShutdownShader();
}


bool FireShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, 
							 XMMATRIX projectionMatrix, ID3D11ShaderResourceView* fireTexture, 
							 ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, float frameTime,
							 XMFLOAT3 scrollSpeeds, XMFLOAT3 scales, XMFLOAT2 distortion1, XMFLOAT2 distortion2,
							 XMFLOAT2 distortion3, float distortionScale, float distortionBias)
{
	// 렌더링에 사용할 셰이더 매개 변수를 설정합니다.
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, fireTexture, noiseTexture, alphaTexture, 
								 frameTime, scrollSpeeds, scales, distortion1, distortion2, distortion3, distortionScale, 
								 distortionBias))
	{
		return false;
	}

	// 설정된 버퍼를 셰이더로 렌더링한다.
	RenderShader(deviceContext, indexCount);

	return true;
}


bool FireShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* vsFilename, const WCHAR* psFilename)
{
	HRESULT result;
	Microsoft::WRL::ComPtr<ID3D10Blob> errorMessage;

	// 버텍스 쉐이더 코드를 컴파일한다.
	Microsoft::WRL::ComPtr<ID3D10Blob> vertexShaderBuffer;
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "FireVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, vertexShaderBuffer.ReleaseAndGetAddressOf(), errorMessage.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage.Get(), hwnd, vsFilename);
		}
		// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 픽셀 쉐이더 코드를 컴파일한다.
	Microsoft::WRL::ComPtr<ID3D10Blob> pixelShaderBuffer;
	result = D3DCompileFromFile(psFilename, NULL, NULL, "FirePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, pixelShaderBuffer.ReleaseAndGetAddressOf(), errorMessage.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		// 셰이더 컴파일 실패시 오류메시지를 출력합니다.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage.Get(), hwnd, psFilename);
		}
		// 컴파일 오류가 아니라면 셰이더 파일을 찾을 수 없는 경우입니다.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 버퍼로부터 정점 셰이더를 생성한다.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, m_vertexShader.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// 버퍼에서 픽셀 쉐이더를 생성합니다.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, m_pixelShader.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// 정점 입력 레이아웃 구조체를 설정합니다.
	// 이 설정은 ModelClass와 셰이더의 VertexType 구조와 일치해야합니다.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// 레이아웃의 요소 수를 가져옵니다.
	UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 정점 입력 레이아웃을 만듭니다.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), m_layout.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

    // 버텍스 쉐이더에있는 동적 행렬 상수 버퍼의 설명을 설정합니다.
	D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 이 클래스 내에서 정점 셰이더 상수 버퍼에 액세스 할 수 있도록 행렬 버퍼 포인터를 만듭니다.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, m_matrixBuffer.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

    // 버텍스 쉐이더에있는 동적 노이즈 상수 버퍼의 설명을 설정합니다.
	D3D11_BUFFER_DESC noiseBufferDesc;
    noiseBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	noiseBufferDesc.ByteWidth = sizeof(NoiseBufferType);
    noiseBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    noiseBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    noiseBufferDesc.MiscFlags = 0;
	noiseBufferDesc.StructureByteStride = 0;

	// 이 클래스 내에서 정점 셰이더 상수 버퍼에 액세스 할 수 있도록 노이즈 버퍼 포인터를 만듭니다.
	result = device->CreateBuffer(&noiseBufferDesc, NULL, m_noiseBuffer.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// 텍스처 샘플러 상태 설명을 만듭니다.
	D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// 텍스처 샘플러 상태를 만듭니다.
    result = device->CreateSamplerState(&samplerDesc, m_sampleState.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	// 클램프 샘플러에 대한 두 번째 텍스처 샘플러 상태 설명을 만듭니다.
	D3D11_SAMPLER_DESC samplerDesc2;
    samplerDesc2.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc2.MipLODBias = 0.0f;
    samplerDesc2.MaxAnisotropy = 1;
    samplerDesc2.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc2.BorderColor[0] = 0;
	samplerDesc2.BorderColor[1] = 0;
	samplerDesc2.BorderColor[2] = 0;
	samplerDesc2.BorderColor[3] = 0;
    samplerDesc2.MinLOD = 0;
    samplerDesc2.MaxLOD = D3D11_FLOAT32_MAX;

	// 텍스처 샘플러 상태를 만듭니다.
    result = device->CreateSamplerState(&samplerDesc2, m_sampleState2.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

    // 픽셀 쉐이더에있는 동적 왜곡 상수 버퍼의 설명을 설정합니다.
	D3D11_BUFFER_DESC distortionBufferDesc;
    distortionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	distortionBufferDesc.ByteWidth = sizeof(DistortionBufferType);
    distortionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    distortionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    distortionBufferDesc.MiscFlags = 0;
	distortionBufferDesc.StructureByteStride = 0;

	// 이 클래스 내에서 픽셀 쉐이더 상수 버퍼에 액세스 할 수 있도록 왜곡 버퍼 포인터를 만듭니다.
	result = device->CreateBuffer(&distortionBufferDesc, NULL, m_distortionBuffer.ReleaseAndGetAddressOf());
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void FireShaderClass::ShutdownShader()
{
	// 왜곡 상수 버퍼를 해제합니다.
	m_distortionBuffer.Reset();

	// 두 번째 샘플러 상태를 해제합니다.
	m_sampleState2.Reset();

	// 샘플러 상태를 해제한다.
	m_sampleState.Reset();

	// 잡음 상수 버퍼를 해제한다.
	m_noiseBuffer.Reset();

	// 행렬 상수 버퍼를 해제합니다.
	m_matrixBuffer.Reset();

	// 레이아웃을 해제합니다.
	m_layout.Reset();

	// 픽셀 쉐이더를 해제합니다.
	m_pixelShader.Reset();

	// 버텍스 쉐이더를 해제합니다.
	m_vertexShader.Reset();
}


void FireShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
{
	// 에러 메시지를 출력창에 표시합니다.
	OutputDebugStringA(reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));

	// 컴파일 에러가 있음을 팝업 메세지로 알려줍니다.
	MessageBox(hwnd, L"Error compiling shader.", shaderFilename, MB_OK);
}


bool FireShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
										  XMMATRIX projectionMatrix, ID3D11ShaderResourceView* fireTexture, 
										  ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, 
										  float frameTime, XMFLOAT3 scrollSpeeds, XMFLOAT3 scales, XMFLOAT2 distortion1, 
										  XMFLOAT2 distortion2, XMFLOAT2 distortion3, float distortionScale, 
										  float distortionBias)
{
	// 행렬을 transpose하여 셰이더에서 사용할 수 있게 합니다
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// 상수 버퍼의 내용을 쓸 수 있도록 잠급니다.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(deviceContext->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// 상수 버퍼에 행렬을 복사합니다.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// 상수 버퍼의 잠금을 풉니다.
	deviceContext->Unmap(m_matrixBuffer.Get(), 0);

	// 정점 셰이더에서의 상수 버퍼의 위치를 설정합니다.
	unsigned int bufferNumber = 0;

	// 마지막으로 정점 셰이더의 상수 버퍼를 바뀐 값으로 바꿉니다.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, m_matrixBuffer.GetAddressOf());

	// 쓸 수 있도록 노이즈 상수 버퍼를 잠급니다.
	if(FAILED(deviceContext->Map(m_noiseBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// 노이즈 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	NoiseBufferType* dataPtr2 = (NoiseBufferType*)mappedResource.pData;

	// 데이터를 노이즈 상수 버퍼에 복사합니다.
	dataPtr2->frameTime = frameTime;
	dataPtr2->scrollSpeeds = scrollSpeeds;
	dataPtr2->scales = scales;
	dataPtr2->padding = 0.0f;

	// 노이즈 상수 버퍼의 잠금을 해제합니다.
    deviceContext->Unmap(m_noiseBuffer.Get(), 0);

	// 버텍스 쉐이더에서 노이즈 상수 버퍼의 위치를 ​​설정합니다.
	bufferNumber = 1;

	// 이제 버텍스 쉐이더에서 업데이트 된 값으로 노이즈 상수 버퍼를 설정합니다.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, m_noiseBuffer.GetAddressOf());

	// 픽셀 쉐이더에 3 개의 쉐이더 텍스처 리소스를 설정합니다.
	deviceContext->PSSetShaderResources(0, 1, &fireTexture);
	deviceContext->PSSetShaderResources(1, 1, &noiseTexture);
	deviceContext->PSSetShaderResources(2, 1, &alphaTexture);

	// 쓸 수 있도록 왜곡 상수 버퍼를 잠급니다.
	if(FAILED(deviceContext->Map(m_distortionBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// 왜곡 상수 버퍼의 데이터에 대한 포인터를 가져옵니다.
	DistortionBufferType* dataPtr3 = (DistortionBufferType*)mappedResource.pData;

	// 왜곡 상수 버퍼에 데이터를 복사합니다.
	dataPtr3->distortion1 = distortion1;
	dataPtr3->distortion2 = distortion2;
	dataPtr3->distortion3 = distortion3;
	dataPtr3->distortionScale = distortionScale;
	dataPtr3->distortionBias = distortionBias;

	// 왜곡 상수 버퍼의 잠금을 해제합니다.
    deviceContext->Unmap(m_distortionBuffer.Get(), 0);

	// 픽셀 쉐이더에서 왜곡 상수 버퍼의 위치를 ​​설정합니다.
	bufferNumber = 0;

	// 이제 픽셀 쉐이더에서 왜곡 상수 버퍼를 업데이트 된 값으로 설정합니다.
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, m_distortionBuffer.GetAddressOf());

	return true;
}


void FireShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// 정점 입력 레이아웃을 설정합니다.
	deviceContext->IASetInputLayout(m_layout.Get());

	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	deviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// 픽셀 쉐이더에서 샘플러 상태를 설정합니다.
	deviceContext->PSSetSamplers(0, 1, m_sampleState.GetAddressOf());
	deviceContext->PSSetSamplers(1, 1, m_sampleState2.GetAddressOf());

	// 삼각형을 렌더링합니다.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
