#include "stdafx.h"
#include "skydomeclass.h"

#include <fstream>
#include <utility>
#include <vector>
using namespace std;


SkyDomeClass::SkyDomeClass()
{
}


SkyDomeClass::~SkyDomeClass()
{
	Shutdown();
}


bool SkyDomeClass::Initialize(ID3D11Device* device)
{
	Shutdown();
	if (device == nullptr)
	{
		return false;
	}

	// ��ī�� �� �� ������ �о�ɴϴ�.
	if(!LoadSkyDomeModel("./data/skydome.txt"))
	{
		return false;
	}

	// ��ī�� ���� ������ �ε��ϰ� �������� ���� �ε��� ���۸� �ε��մϴ�.
	if(!InitializeBuffers(device))
	{
		Shutdown();
		return false;
	}

	// ��ī�� �� ����⿡ ������ �����մϴ�.
	m_apexColor = XMFLOAT4(0.0f, 0.15f, 0.66f, 1.0f);
	
	// ��ī�� ���� �߽ɿ� ������ �����մϴ�.
	m_centerColor =  XMFLOAT4(0.2f, 0.38f, 0.66f, 1.0f);
	
	return true;
}


void SkyDomeClass::Shutdown()
{
	// ��ī�� �� �������� ���� ���� �� �ε��� ���۸� �����մϴ�.
	ReleaseBuffers();

	// ��ī�� �� ���� �����մϴ�.
	ReleaseSkyDomeModel();
}


void SkyDomeClass::Render(ID3D11DeviceContext* deviceContext)
{
	// ��ī�� ���� ������ �մϴ�.
	RenderBuffers(deviceContext);
}


int SkyDomeClass::GetIndexCount()
{
	return m_indexCount;
}


XMFLOAT4 SkyDomeClass::GetApexColor()
{
	return m_apexColor;
}


XMFLOAT4 SkyDomeClass::GetCenterColor()
{
	return m_centerColor;
}


bool SkyDomeClass::LoadSkyDomeModel(const char* filename)
{
	if (filename == nullptr)
	{
		return false;
	}

	ifstream fin(filename);
	
	// ������ �� �� ������ �����մϴ�.
	if(fin.fail())
	{
		return false;
	}

	// ���ؽ� ī��Ʈ�� ������ �д´�.
	char input = 0;
	while (fin.get(input) && input != ':') {}
	if (!fin)
	{
		return false;
	}

	// ���ؽ� ī��Ʈ�� �д´�.
	int vertexCount = 0;
	if (!(fin >> vertexCount) || vertexCount <= 0)
	{
		return false;
	}

	std::vector<ModelType> model(static_cast<std::size_t>(vertexCount));

	// �������� ���� �κб��� �д´�.
	while (fin.get(input) && input != ':') {}
	if (!fin)
	{
		return false;
	}
	fin.get(input);
	fin.get(input);

	// ���ؽ� �����͸� �н��ϴ�.
	for(int i = 0; i < vertexCount; ++i)
	{
		ModelType& value = model[static_cast<std::size_t>(i)];
		if (!(fin >> value.x >> value.y >> value.z >>
			value.tu >> value.tv >> value.nx >> value.ny >> value.nz))
		{
			return false;
		}
	}

	m_vertexCount = vertexCount;
	m_indexCount = vertexCount;
	m_model = std::move(model);
	return true;
}


void SkyDomeClass::ReleaseSkyDomeModel()
{
	m_model.clear();
	m_vertexCount = 0;
	m_indexCount = 0;
}


bool SkyDomeClass::InitializeBuffers(ID3D11Device* device)
{
	if (device == nullptr || m_model.size() != static_cast<std::size_t>(m_vertexCount))
	{
		return false;
	}
	std::vector<VertexType> vertices(static_cast<std::size_t>(m_vertexCount));
	std::vector<std::uint32_t> indices(static_cast<std::size_t>(m_indexCount));

	// ���� �迭�� �ε��� �迭�� �����ͷ� �ε��մϴ�.
	for(int i=0; i<m_vertexCount; i++)
	{
		vertices[static_cast<std::size_t>(i)].position = XMFLOAT3(m_model[static_cast<std::size_t>(i)].x, m_model[static_cast<std::size_t>(i)].y, m_model[static_cast<std::size_t>(i)].z);
		indices[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(i);
	}

	// ���� ������ ����ü�� �����Ѵ�.
	D3D11_BUFFER_DESC vertexBufferDesc{};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexType) * m_vertexCount);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// subresource ������ ���� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA vertexData{};
	vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// ���� ��ħ�� ���� ���۸� ����ϴ�.
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	if(FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer.GetAddressOf())))
	{
		return false;
	}

	// �ε��� ������ ����ü�� �����մϴ�.
	D3D11_BUFFER_DESC indexBufferDesc{};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(std::uint32_t) * m_indexCount);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// ���� ���ҽ� ������ �ε��� �����Ϳ� ���� �����͸� �����մϴ�.
	D3D11_SUBRESOURCE_DATA indexData{};
	indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// �ε��� ���۸� ����ϴ�.
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	if(FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer.GetAddressOf())))
	{
		return false;
	}

	m_vertexBuffer = std::move(vertexBuffer);
	m_indexBuffer = std::move(indexBuffer);

	return true;
}


void SkyDomeClass::ReleaseBuffers()
{
	m_indexBuffer.Reset();
	m_vertexBuffer.Reset();
}


void SkyDomeClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	// ���� ���� ���� �� �������� �����մϴ�.
    unsigned int stride = sizeof(VertexType); 
	unsigned int offset = 0;
    
	// ������ �� �� �ֵ��� �Է� ����������� ���� ���۸� Ȱ������ �����մϴ�.
	ID3D11Buffer* vertexBuffer = m_vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // ������ �� �� �ֵ��� �Է� ����������� �ε��� ���۸� Ȱ������ �����մϴ�.
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // �� ������ ���ۿ��� �������Ǿ���ϴ� ������Ƽ�� ������ �����մϴ�.�� ��쿡�� �ﰢ���Դϴ�.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
