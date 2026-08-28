#include "stdafx.h"
#include "Engine/Rendering/Textures/TextureResources.h"
#include "Engine/Rendering/Geometry/TerrainClass.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <utility>
#include <vector>


TerrainClass::TerrainClass()
{
}


TerrainClass::~TerrainClass()
{
	Shutdown();
}


bool TerrainClass::Initialize(ID3D11Device* device, const char* heightMapFilename, const WCHAR* textureFilename)
{
	Shutdown();
	if (device == nullptr || heightMapFilename == nullptr || textureFilename == nullptr)
	{
		return false;
	}

	// 지형의 높이 맵을 로드합니다.
	if(!LoadHeightMap(heightMapFilename))
	{
		return false;
	}

	// 높이 맵의 높이를 표준화합니다.
	NormalizeHeightMap();

	// 지형 데이터의 법선을 계산합니다.
	if(!CalculateNormals())
	{
		Shutdown();
		return false;
	}

	// 텍스처 좌표를 계산합니다.
	CalculateTextureCoordinates();

	// 텍스처를 로드합니다.
	if(!LoadTexture(device, textureFilename))
	{
		Shutdown();
		return false;
	}

	// 지형에 대한 지오 메트릭을 포함하는 정점 및 인덱스 버퍼를 초기화합니다.
	if (!InitializeBuffers(device))
	{
		Shutdown();
		return false;
	}
	return true;
}


void TerrainClass::Shutdown()
{
	// 텍스처를 해제합니다.
	ReleaseTexture();

	// 버텍스와 인덱스 버퍼를 해제합니다.
	ShutdownBuffers();

	// 높이맵 데이터를 해제합니다.
	ShutdownHeightMap();
}


void TerrainClass::Render(ID3D11DeviceContext* deviceContext)
{
	// 그리기를 준비하기 위해 그래픽 파이프 라인에 꼭지점과 인덱스 버퍼를 놓습니다.
	RenderBuffers(deviceContext);
}


int TerrainClass::GetIndexCount()
{
	return m_indexCount;
}


ID3D11ShaderResourceView* TerrainClass::GetTexture()
{
	return m_texture ? m_texture->GetShaderResourceView() : nullptr;
}


bool TerrainClass::LoadHeightMap(const char* filename)
{
	if (filename == nullptr)
	{
		return false;
	}

	// 바이너리 모드로 높이맵 파일을 엽니다.
	FILE* filePtr = nullptr;
	if(fopen_s(&filePtr, filename, "rb") != 0)
	{
		return false;
	}
	std::unique_ptr<FILE, int(*)(FILE*)> file(filePtr, &fclose);

	// 파일 헤더를 읽습니다.
	BITMAPFILEHEADER bitmapFileHeader{};
	if(fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, file.get()) != 1)
	{
		return false;
	}

	// 비트맵 정보 헤더를 읽습니다.
	BITMAPINFOHEADER bitmapInfoHeader{};
	if(fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, file.get()) != 1)
	{
		return false;
	}

	if (bitmapFileHeader.bfType != 0x4D42 || bitmapInfoHeader.biWidth <= 1 ||
		bitmapInfoHeader.biHeight <= 1 || bitmapInfoHeader.biBitCount != 24 ||
		bitmapInfoHeader.biCompression != BI_RGB)
	{
		return false;
	}

	const int terrainWidth = bitmapInfoHeader.biWidth;
	const int terrainHeight = bitmapInfoHeader.biHeight;
	const std::size_t width = static_cast<std::size_t>(terrainWidth);
	const std::size_t height = static_cast<std::size_t>(terrainHeight);
	if (width > (std::numeric_limits<std::size_t>::max)() / height ||
		width * height > (std::numeric_limits<std::size_t>::max)() / 3u)
	{
		return false;
	}

	// 비트맵 이미지 데이터의 크기를 계산합니다.
	const std::size_t imageSize = width * height * 3u;
	std::vector<unsigned char> bitmapImage(imageSize);

	// 비트맵 데이터의 시작 부분으로 이동합니다.
	if (fseek(file.get(), static_cast<long>(bitmapFileHeader.bfOffBits), SEEK_SET) != 0)
	{
		return false;
	}

	// 비트맵 이미지 데이터를 읽습니다.
	if(fread(bitmapImage.data(), 1, imageSize, file.get()) != imageSize)
	{
		return false;
	}

	std::vector<HeightMapType> heightMap(width * height);

	// 이미지 데이터 버퍼의 위치를 ​​초기화합니다.
	int k = 0;

	// 이미지 데이터를 높이 맵으로 읽어들입니다.
	for(int j = 0; j < terrainHeight; ++j)
	{
		for(int i = 0; i < terrainWidth; ++i)
		{
			unsigned char heightValue = bitmapImage[static_cast<std::size_t>(k)];

			const std::size_t index = (static_cast<std::size_t>(terrainWidth) * j) + i;

			heightMap[index].x = static_cast<float>(i);
			heightMap[index].y = static_cast<float>(heightValue);
			heightMap[index].z = static_cast<float>(j);

			k+=3;
		}
	}

	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;
	m_heightMap = std::move(heightMap);
	return true;
}


void TerrainClass::NormalizeHeightMap()
{
	for(int j=0; j<m_terrainHeight; j++)
	{
		for(int i=0; i<m_terrainWidth; i++)
		{
			m_heightMap[(m_terrainWidth * j) + i].y /= 15.0f;
		}
	}
}


bool TerrainClass::CalculateNormals()
{
	if (m_terrainWidth < 2 || m_terrainHeight < 2 ||
		m_heightMap.size() != static_cast<std::size_t>(m_terrainWidth) * m_terrainHeight)
	{
		return false;
	}

	int index1 = 0;
	int index2 = 0;
	int index3 = 0;
	int index = 0;
	int count = 0;
	float vertex1[3] = { 0.f, 0.f, 0.f };
	float vertex2[3] = { 0.f, 0.f, 0.f };
	float vertex3[3] = { 0.f, 0.f, 0.f };
	float vector1[3] = { 0.f, 0.f, 0.f };
	float vector2[3] = { 0.f, 0.f, 0.f };
	float sum[3] = { 0.f, 0.f, 0.f };
	float length = 0.0f;


	// 정규화되지 않은 법선 벡터를 저장할 임시 배열을 만듭니다.
	std::vector<VectorType> normals(
		static_cast<std::size_t>(m_terrainHeight - 1) * (m_terrainWidth - 1));

	// 메쉬의 모든면을 살펴보고 법선을 계산합니다.
	for(int j=0; j<(m_terrainHeight-1); j++)
	{
		for(int i=0; i<(m_terrainWidth-1); i++)
		{
			index1 = (j * m_terrainWidth) + i;
			index2 = (j * m_terrainWidth) + (i+1);
			index3 = ((j+1) * m_terrainWidth) + i;

			// 표면에서 세 개의 꼭지점을 가져옵니다.
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;

			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;

			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			// 표면의 두 벡터를 계산합니다.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainWidth-1)) + i;

			// 이 두 법선에 대한 정규화되지 않은 값을 얻기 위해 두 벡터의 외적을 계산합니다.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// 이제 모든 정점을 살펴보고 각면의 평균을 취합니다.
	// 정점이 닿아 그 정점에 대한 평균 평균값을 얻는다.
	for(int j=0; j<m_terrainHeight; j++)
	{
		for(int i=0; i<m_terrainWidth; i++)
		{
			// 합계를 초기화합니다.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// 카운트를 초기화합니다.
			count = 0;

			// 왼쪽 아래면.
			if(((i-1) >= 0) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainWidth-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// 오른쪽 아래 면.
			if((i < (m_terrainWidth-1)) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainWidth-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// 왼쪽 위 면.
			if(((i-1) >= 0) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainWidth-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// 오른쪽 위 면.
			if((i < (m_terrainWidth-1)) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainWidth-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// 이 정점에 닿는면의 평균을 취합니다.
			if (count == 0)
			{
				return false;
			}
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// 이 법선의 길이를 계산합니다.
			length = std::sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));
			if (length <= 0.0f)
			{
				return false;
			}

			// 높이 맵 배열의 정점 위치에 대한 인덱스를 가져옵니다.
			index = (j * m_terrainWidth) + i;

			// 이 정점의 최종 공유 법선을 표준화하여 높이 맵 배열에 저장합니다.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	return true;
}


void TerrainClass::ShutdownHeightMap()
{
	m_heightMap.clear();
	m_terrainWidth = 0;
	m_terrainHeight = 0;
}


void TerrainClass::CalculateTextureCoordinates()
{
	if (m_terrainWidth <= 0 || m_terrainHeight <= 0 || m_heightMap.empty())
	{
		return;
	}

	// 텍스처 좌표를 얼마나 많이 증가 시킬지 계산합니다.
	float incrementValue = (float)TEXTURE_REPEAT / (float)m_terrainWidth;

	// 텍스처를 반복 할 횟수를 계산합니다.
	int incrementCount = m_terrainWidth / TEXTURE_REPEAT;

	// tu 및 tv 좌표 값을 초기화합니다.
	float tuCoordinate = 0.0f;
	float tvCoordinate = 1.0f;

	//  tu 및 tv 좌표 인덱스를 초기화합니다.
	int tuCount = 0;
	int tvCount = 0;

	// 전체 높이 맵을 반복하고 각 꼭지점의 tu 및 tv 텍스처 좌표를 계산합니다.
	for(int j=0; j<m_terrainHeight; j++)
	{
		for(int i=0; i<m_terrainWidth; i++)
		{
			// 높이 맵에 텍스처 좌표를 저장한다.
			m_heightMap[(m_terrainWidth * j) + i].tu = tuCoordinate;
			m_heightMap[(m_terrainWidth * j) + i].tv = tvCoordinate;

			// tu 텍스처 좌표를 증가 값만큼 증가시키고 인덱스를 1 씩 증가시킨다.
			tuCoordinate += incrementValue;
			tuCount++;

			// 텍스처의 오른쪽 끝에 있는지 확인하고, 그렇다면 처음부터 다시 시작하십시오.
			if(tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}

		// tv 텍스처 좌표를 증가 값만큼 증가시키고 인덱스를 1 씩 증가시킵니다.
		tvCoordinate -= incrementValue;
		tvCount++;

		// 텍스처의 상단에 있는지 확인하고, 그렇다면 하단에서 다시 시작합니다.
		if(tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}
}


bool TerrainClass::LoadTexture(ID3D11Device* device, const WCHAR* filename)
{
	auto texture = Engine::Rendering::MakeTexture<Engine::Rendering::DdsTexture>(device, filename);
	if (!texture)
	{
		return false;
	}

	m_texture = std::move(texture);
	return true;
}


void TerrainClass::ReleaseTexture()
{
	m_texture.reset();
}


bool TerrainClass::InitializeBuffers(ID3D11Device* device)
{
	if (device == nullptr || m_terrainWidth < 2 || m_terrainHeight < 2 || m_heightMap.empty())
	{
		return false;
	}

	float tu = 0.0f;
	float tv = 0.0f;

	// 지형 메쉬의 정점 수를 계산합니다.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// 인덱스 수를 꼭지점 수와 같게 설정합니다.
	m_indexCount = m_vertexCount;

	std::vector<VertexType> vertices(static_cast<std::size_t>(m_vertexCount));
	std::vector<std::uint32_t> indices(static_cast<std::size_t>(m_indexCount));

	// 정점 배열에 대한 인덱스를 초기화합니다.
	int index = 0;

	// 지형 데이터로 정점 및 인덱스 배열을 로드합니다.
	for(int j=0; j<(m_terrainHeight-1); j++)
	{
		for(int i=0; i<(m_terrainWidth-1); i++)
		{
			int index1 = (m_terrainWidth * j) + i;          // 왼쪽 아래.
			int index2 = (m_terrainWidth * j) + (i+1);      // 오른쪽 아래.
			int index3 = (m_terrainWidth * (j+1)) + i;      // 왼쪽 위.
			int index4 = (m_terrainWidth * (j+1)) + (i+1);  // 오른쪽 위.

			// 왼쪽 위.
			tv = m_heightMap[index3].tv;

			// 상단 가장자리를 덮도록 텍스처 좌표를 수정합니다.
			if(tv == 1.0f) { tv = 0.0f; }

			vertices[index].position = XMFLOAT3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].texture = XMFLOAT2(m_heightMap[index3].tu, tv);
			vertices[index].normal = XMFLOAT3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
			indices[index] = index;
			index++;

			// 오른쪽 위.
			tu = m_heightMap[index4].tu;
			tv = m_heightMap[index4].tv;

			// 위쪽과 오른쪽 가장자리를 덮도록 텍스처 좌표를 수정합니다.
			if(tu == 0.0f) { tu = 1.0f; }
			if(tv == 1.0f) { tv = 0.0f; }

			vertices[index].position = XMFLOAT3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].texture = XMFLOAT2(tu, tv);
			vertices[index].normal = XMFLOAT3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			indices[index] = index;
			index++;

			// 왼쪽 아래.
			vertices[index].position = XMFLOAT3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].texture = XMFLOAT2(m_heightMap[index1].tu, m_heightMap[index1].tv);
			vertices[index].normal = XMFLOAT3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			indices[index] = index;
			index++;

			// 왼쪽 아래.
			vertices[index].position = XMFLOAT3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].texture = XMFLOAT2(m_heightMap[index1].tu, m_heightMap[index1].tv);
			vertices[index].normal = XMFLOAT3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			indices[index] = index;
			index++;

			// 오른쪽 위.
			tu = m_heightMap[index4].tu;
			tv = m_heightMap[index4].tv;

			// 위쪽과 오른쪽 가장자리를 덮도록 텍스처 좌표를 수정합니다.
			if(tu == 0.0f) { tu = 1.0f; }
			if(tv == 1.0f) { tv = 0.0f; }

			vertices[index].position = XMFLOAT3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].texture = XMFLOAT2(tu, tv);
			vertices[index].normal = XMFLOAT3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			indices[index] = index;
			index++;

			// 오른쪽 아래.
			tu = m_heightMap[index2].tu;

			// 오른쪽 가장자리를 덮도록 텍스처 좌표를 수정합니다.
			if(tu == 0.0f) { tu = 1.0f; }

			vertices[index].position = XMFLOAT3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].texture = XMFLOAT2(tu, m_heightMap[index2].tv);
			vertices[index].normal = XMFLOAT3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
			indices[index] = index;
			index++;
		}
	}

	// 정적 정점 버퍼의 구조체를 설정한다.
	D3D11_BUFFER_DESC vertexBufferDesc{};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource 구조에 정점 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA vertexData{};
    vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 정점 버퍼를 만듭니다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	if(FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer.GetAddressOf())))
	{
		return false;
	}

	// 정적 인덱스 버퍼의 구조체를 설정합니다.
	D3D11_BUFFER_DESC indexBufferDesc{};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(std::uint32_t) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 하위 리소스 구조에 인덱스 데이터에 대한 포인터를 제공합니다.
	D3D11_SUBRESOURCE_DATA indexData{};
    indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 만듭니다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	if(FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer.GetAddressOf())))
	{
		return false;
	}

	m_vertexBuffer = std::move(vertexBuffer);
	m_indexBuffer = std::move(indexBuffer);

	return true;
}


void TerrainClass::ShutdownBuffers()
{
	m_indexBuffer.Reset();
	m_vertexBuffer.Reset();
	m_vertexCount = 0;
	m_indexCount = 0;
}


void TerrainClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	// 정점 버퍼 보폭 및 오프셋을 설정합니다.
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	// 렌더링 할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성으로 설정합니다.
	ID3D11Buffer* vertexBuffer = m_vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // 렌더링 할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성으로 설정합니다.
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // 이 꼭지점 버퍼에서 렌더링 되어야 하는 프리미티브 유형을 설정합니다. 이 경우에는 삼각형입니다.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void TerrainClass::SetWorldOffset(const XMFLOAT3& worldOffset) noexcept
{
	m_worldOffset = worldOffset;
}

float TerrainClass::GetHeight(float x, float z) const noexcept
{
	if (m_heightMap.empty())
	{
		return m_worldOffset.y;
	}

	const int mapX = static_cast<int>(std::floor(x - m_worldOffset.x));
	const int mapZ = static_cast<int>(std::floor(z - m_worldOffset.z));

	if (mapX < 0 || mapX >= m_terrainWidth || mapZ < 0 || mapZ >= m_terrainHeight)
	{
		return m_worldOffset.y;
	}

	const int index = (m_terrainWidth * mapZ) + mapX;
	return m_heightMap[index].y + m_worldOffset.y;
}
