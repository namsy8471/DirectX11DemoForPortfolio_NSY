#include "stdafx.h"
#include "Engine/Rendering/Textures/TextureResources.h"
#include "Engine/Rendering/Geometry/TerrainClass.h"
#include "Engine/Rendering/Geometry/foliageclass.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <utility>
#include <vector>


FoliageClass::FoliageClass()
{
}


FoliageClass::~FoliageClass()
{
	Shutdown();
}

void FoliageClass::SetPosition(float x, float y, float z)
{
	m_position = { x, y, z };
}


void FoliageClass::GetPosition(float& x, float& y, float& z)
{
	x = m_position.x;
	y = m_position.y;
	z = m_position.z;
}

bool FoliageClass::Initialize(ID3D11Device* device, const WCHAR* textureFilename, int fCount)
{
	Shutdown();
	if (device == nullptr || textureFilename == nullptr || fCount <= 0)
	{
		return false;
	}

	// 단풍 수를 설정합니다.
	m_foliageCount = fCount;

	// 단풍의 위치를 ​​생성합니다.
	if(!GeneratePositions())
	{
		return false;
	}

	// 단풍 모델의 지오메트리를 포함하는 정점과 인스턴스 버퍼를 초기화합니다.
	if(!InitializeBuffers(device))
	{
		Shutdown();
		return false;
	}

	//이 모델의 텍스처를 로드합니다.
	if(!LoadTexture(device, textureFilename))
	{
		Shutdown();
		return false;
	}

	// 초기 바람 회전과 방향을 설정합니다.
	m_windRotation = 0.0f;
	m_windDirection = 1;

	return true;
}


void FoliageClass::Shutdown()
{
	// 모델 텍스처를 릴리즈한다.
	ReleaseTexture();

	// 버텍스와 인스턴스 버퍼를 해제한다.
	ShutdownBuffers();

	m_foliageArray.clear();
	m_foliageCount = 0;
}


void FoliageClass::Render(ID3D11DeviceContext* deviceContext)
{
	// 버텍스와 인스턴스 버퍼를 그래픽 파이프 라인에 배치하여 그리기를 준비합니다.
	RenderBuffers(deviceContext);
}


bool FoliageClass::Frame(XMFLOAT3 cameraPosition, ID3D11DeviceContext* deviceContext, float frameTime, TerrainClass* terrain)
{
	if (deviceContext == nullptr || !m_instanceBuffer ||
		m_foliageArray.size() != static_cast<std::size_t>(m_foliageCount) ||
		m_Instances.size() != static_cast<std::size_t>(m_foliageCount))
	{
		return false;
	}

	XMMATRIX rotateMatrix, translationMatrix, rotateMatrix2, finalMatrix;
	XMFLOAT3 modelPosition = { 0.0f, 0.0f, 0.0f };
	float rotation = 0.0f;
	float windRotation = 0.0f;

	// 바람 회전을 업데이트합니다.
	if(m_windDirection == 1)
	{
		m_windRotation += 0.1f * frameTime;
		if(m_windRotation > 10.0f)
		{
			m_windDirection = 2;
		}
	}
	else
	{
		m_windRotation -= 0.1f * frameTime;
		if(m_windRotation < -10.0f)
		{
			m_windDirection = 1;
		}
	}

	// 업데이트 된 위치로 인스턴스 버퍼를 로드합니다.
	for(int i=0; i<m_foliageCount; i++)
	{
		// 이 조각의 위치를 ​​얻는다.
		modelPosition.x = m_foliageArray[i].x;
		modelPosition.z = m_foliageArray[i].z;

		// 지형 높이를 가져와서 풀이 지형 표면 위에 위치하도록 설정
		if (terrain)
		{
			modelPosition.y = terrain->GetHeight(modelPosition.x, modelPosition.z) + 0.1f;  // 지형 위 0.1 단위
		}
		else
		{
			modelPosition.y = -2.5f;  // 지형이 없으면 기본값
		}

		// 아크 탄젠트 함수를 사용하여 현재 카메라 위치를 향하도록 빌보드 모델에 적용해야하는 회전을 계산합니다.
		double angle = atan2(modelPosition.x - cameraPosition.x, modelPosition.z - cameraPosition.z) * (180.0 / XM_PI);

		// 회전을 라디안으로 변환합니다.
		rotation = (float)angle * 0.0174532925f;

		// 빌보드의 X 회전을 설정합니다.
		rotateMatrix = XMMatrixRotationY(rotation);

		// 단풍의 바람 회전을 얻는다.
		windRotation = m_windRotation * 0.0174532925f;

		// 바람 회전을 설정합니다.
		rotateMatrix2 = XMMatrixRotationX(windRotation);

		// 변환 행렬을 설정합니다.
		translationMatrix = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);

		// 최종 행렬을 만들고 인스턴스 배열에 저장합니다.
		finalMatrix = XMMatrixMultiply(rotateMatrix, rotateMatrix2);
		m_Instances[i].matrix = XMMatrixMultiply(finalMatrix, translationMatrix);
	}

	// 쓸 수 있도록 인스턴스 버퍼를 잠급니다.
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	if(FAILED(deviceContext->Map(m_instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// 인스턴스 버퍼의 데이터에 대한 포인터를 가져옵니다.
	InstanceType* instancesPtr = (InstanceType*)mappedResource.pData;

	// 인스턴스 배열을 인스턴스 버퍼에 복사합니다.
	memcpy(instancesPtr, m_Instances.data(), (sizeof(InstanceType) * m_foliageCount));

	// 인스턴스 버퍼를 잠금 해제합니다.
	deviceContext->Unmap(m_instanceBuffer.Get(), 0);

	return true;
}


int FoliageClass::GetVertexCount()
{
	return m_vertexCount;
}


int FoliageClass::GetInstanceCount()
{
	return m_instanceCount;
}


ID3D11ShaderResourceView* FoliageClass::GetTexture()
{
	return m_texture ? m_texture->GetShaderResourceView() : nullptr;
}


bool FoliageClass::InitializeBuffers(ID3D11Device* device)
{
	if (device == nullptr || m_foliageArray.size() != static_cast<std::size_t>(m_foliageCount))
	{
		return false;
	}

	// 정점 배열의 정점 수를 설정합니다.
	m_vertexCount = 6;

	std::vector<VertexType> vertices(static_cast<std::size_t>(m_vertexCount));

	// 정점 배열에 데이터를 로드합니다.
	vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);  // 왼쪽 아래.
	vertices[0].texture = XMFLOAT2(0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // 왼쪽 위.
	vertices[1].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[2].position = XMFLOAT3(1.0f, 0.0f, 0.0f);  // 오른쪽 아래.
	vertices[2].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[3].position = XMFLOAT3(1.0f, 0.0f, 0.0f);  // 오른쪽 아래.
	vertices[3].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[4].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // 왼쪽 위.
	vertices[4].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[5].position = XMFLOAT3(1.0f, 1.0f, 0.0f);  // 오른쪽 위.
	vertices[5].texture = XMFLOAT2(1.0f, 0.0f);

	// 정점 버퍼의 구조체를 설정한다.
	D3D11_BUFFER_DESC vertexBufferDesc{};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexType) * m_vertexCount);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData{};
	vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 마침내 정점 버퍼를 만듭니다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	if(FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer.GetAddressOf())))
	{
		return false;
	}

	// 배열의 인스턴스 수를 설정합니다.
	m_instanceCount = m_foliageCount;

	std::vector<InstanceType> instances(static_cast<std::size_t>(m_instanceCount));

	// 초기 매트릭스를 설정합니다.
	XMMATRIX matrix = XMMatrixIdentity();
	matrix = XMMatrixTranslation(0, -5.f, 0);

	// 데이터로 인스턴스 배열을로드합니다.
	for(int i=0; i<m_instanceCount; i++)
	{
		const std::size_t index = static_cast<std::size_t>(i);
		instances[index].matrix = matrix;
		instances[index].color = XMFLOAT3(m_foliageArray[index].r, m_foliageArray[index].g, m_foliageArray[index].b);
	}

	// 인스턴스 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC instanceBufferDesc{};
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceType) * m_instanceCount);
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	// 하위 리소스 구조에 인스턴스 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA instanceData{};
	instanceData.pSysMem = instances.data();
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	// 인스턴스 버퍼를 만듭니다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> instanceBuffer;
	if(FAILED(device->CreateBuffer(&instanceBufferDesc, &instanceData, instanceBuffer.GetAddressOf())))
	{
		return false;
	}

	m_vertexBuffer = std::move(vertexBuffer);
	m_instanceBuffer = std::move(instanceBuffer);
	m_Instances = std::move(instances);
	return true;
}


void FoliageClass::ShutdownBuffers()
{
	m_instanceBuffer.Reset();
	m_vertexBuffer.Reset();
	m_Instances.clear();
	m_vertexCount = 0;
	m_instanceCount = 0;
}


void FoliageClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	// 버퍼 스트라이드를 설정합니다.
	unsigned int strides[2] = { sizeof(VertexType), sizeof(InstanceType) };

	// 버퍼 오프셋을 설정합니다.
	unsigned int offsets[2] = { 0, 0 };

	// 포인터의 배열을 정점 버퍼와 인스턴스 버퍼로 설정합니다.
	ID3D11Buffer* bufferPointers[2] = { m_vertexBuffer.Get(), m_instanceBuffer.Get() };

	// 렌더링 할 수 있도록 입력 어셈블러에서 꼭지점 버퍼와 인스턴스 버퍼를 활성화로 설정합니다.
	deviceContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

	// 이 꼭지점 버퍼에서 렌더링되어야하는 프리미티브 유형을 설정합니다.이 경우에는 삼각형입니다.
    deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


bool FoliageClass::LoadTexture(ID3D11Device* device, const WCHAR* filename)
{
	auto texture = Engine::Rendering::MakeTexture<Engine::Rendering::DdsTexture>(device, filename);
	if (!texture)
	{
		return false;
	}

	m_texture = std::move(texture);
	return true;
}


void FoliageClass::ReleaseTexture()
{
	m_texture.reset();
}


bool FoliageClass::GeneratePositions()
{
	if (m_foliageCount <= 0)
	{
		return false;
	}
	std::vector<FoliageType> foliage(static_cast<std::size_t>(m_foliageCount));

	// 난수 생성기에 시드합니다.
	srand(static_cast<unsigned int>(time(nullptr)));

	// 각 조각에 임의의 위치와 임의의 색상을 설정합니다.
	for(int i=0; i<m_foliageCount; i++)
	{
		/*m_foliageArray[i].x = ((float)rand() / (float)(RAND_MAX)) * 9.0f - 4.5f;
		m_foliageArray[i].z = ((float)rand() / (float)(RAND_MAX)) * 9.0f - 4.5f;*/


		FoliageType& value = foliage[static_cast<std::size_t>(i)];
		value.x = static_cast<float>((rand() % 100) - 50);
		value.z = static_cast<float>((rand() % 100) - 50);

		float red = ((float)rand() / (float)(RAND_MAX)) * 1.0f;
		float green = ((float)rand() / (float)(RAND_MAX)) * 1.0f;

		value.r = red + 1.0f;
		value.g = green + 0.5f;
		value.b = 0.0f;
	}

	m_foliageArray = std::move(foliage);
	return true;
}
