#include "stdafx.h"
#include "ModelclassForNoiseFilter.h"

#include <fstream>
#include <utility>
#include <vector>
using namespace std;


ModelClassForNoiseFilter::ModelClassForNoiseFilter()
{
}


ModelClassForNoiseFilter::~ModelClassForNoiseFilter()
{
	Shutdown();
}


bool ModelClassForNoiseFilter::Initialize(HWND hwnd, ID3D11Device* device, const char* modelFilename, const WCHAR* textureFilename1, const WCHAR* textureFilename2, 
							const WCHAR* textureFilename3)
{
	Shutdown();
	if (device == nullptr || modelFilename == nullptr || textureFilename1 == nullptr ||
		textureFilename2 == nullptr || textureFilename3 == nullptr)
	{
		return false;
	}

	// �� �����͸� �ε��մϴ�.
	if(!LoadModel(hwnd, modelFilename))
	{
		MessageBox(hwnd, L"Could not Load the model object in Model Class For Noise Filter.", L"Error", MB_OK);
		return false;
	}

	// ���� �� �ε��� ���۸� �ʱ�ȭ�մϴ�.
	if (!InitializeBuffers(device))
	{
		MessageBox(hwnd, L"Could not Initialize buffers in Model Class For Noise Filter.", L"Error", MB_OK);
		Shutdown();
		return false;
	}

	// �� ���� �ؽ�ó�� �ε��մϴ�.
	if (!LoadTextures(device, textureFilename1, textureFilename2, textureFilename3))
	{
		Shutdown();
		return false;
	}
	return true;
}


void ModelClassForNoiseFilter::Shutdown()
{
	// �� �ؽ��ĸ� ��ȯ�մϴ�.
	ReleaseTextures();

	// ���ؽ� �� �ε��� ���۸� �����մϴ�.
	ShutdownBuffers();

	// �� ������ ��ȯ
	ReleaseModel();
}


void ModelClassForNoiseFilter::Render(ID3D11DeviceContext* deviceContext)
{
	// �׸��⸦ �غ��ϱ� ���� �׷��� ������ ���ο� �������� �ε��� ���۸� �����ϴ�.
	RenderBuffers(deviceContext);
}


int ModelClassForNoiseFilter::GetIndexCount()
{
	return m_indexCount;
}


bool ModelClassForNoiseFilter::InitializeBuffers(ID3D11Device* device)
{
	if (device == nullptr || m_model.size() != static_cast<std::size_t>(m_vertexCount))
	{
		return false;
	}

	std::vector<VertexType> vertices(static_cast<std::size_t>(m_vertexCount));
	std::vector<std::uint32_t> indices(static_cast<std::size_t>(m_indexCount));
	for (int i = 0; i < m_vertexCount; ++i)
	{
		const std::size_t index = static_cast<std::size_t>(i);
		vertices[index].position = XMFLOAT3(m_model[index].x, m_model[index].y, m_model[index].z);
		vertices[index].texture = XMFLOAT2(m_model[index].tu, m_model[index].tv);
		indices[index] = static_cast<std::uint32_t>(i);
	}

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexType) * m_vertexCount);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData{};
	vertexData.pSysMem = vertices.data();

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer.GetAddressOf())))
	{
		return false;
	}

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(std::uint32_t) * m_indexCount);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData{};
	indexData.pSysMem = indices.data();

	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer.GetAddressOf())))
	{
		return false;
	}

	m_vertexBuffer = std::move(vertexBuffer);
	m_indexBuffer = std::move(indexBuffer);
	return true;
}


void ModelClassForNoiseFilter::ShutdownBuffers()
{
	m_indexBuffer.Reset();
	m_vertexBuffer.Reset();
}


void ModelClassForNoiseFilter::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	// ���� ������ ������ �������� �����մϴ�.
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	// ������ �� �� �ֵ��� �Է� ����������� ���� ���۸� Ȱ������ �����մϴ�.
	ID3D11Buffer* vertexBuffer = m_vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// ������ �� �� �ֵ��� �Է� ����������� �ε��� ���۸� Ȱ������ �����մϴ�.
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// ���� ���۷� �׸� �⺻���� �����մϴ�. ���⼭�� �ﰢ������ �����մϴ�.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


bool ModelClassForNoiseFilter::LoadTextures(ID3D11Device* device, const WCHAR* textureFilename1, const WCHAR* textureFilename2, const WCHAR* textureFilename3)
{
	auto texture1 = std::make_unique<TextureClass>();
	if(!texture1->Initialize(device, textureFilename1))
	{
		return false;
	}

	auto texture2 = std::make_unique<TextureClass>();
	if(!texture2->Initialize(device, textureFilename2))
	{
		return false;
	}

	auto texture3 = std::make_unique<TextureClass>();
	if(!texture3->Initialize(device, textureFilename3))
	{
		return false;
	}

	m_Texture1 = std::move(texture1);
	m_Texture2 = std::move(texture2);
	m_Texture3 = std::move(texture3);
	return true;
}


void ModelClassForNoiseFilter::ReleaseTextures()
{
	m_Texture1.reset();
	m_Texture2.reset();
	m_Texture3.reset();
}


ID3D11ShaderResourceView* ModelClassForNoiseFilter::GetTexture1()
{
	return m_Texture1 ? m_Texture1->GetTexture() : nullptr;
}


ID3D11ShaderResourceView* ModelClassForNoiseFilter::GetTexture2()
{
	return m_Texture2 ? m_Texture2->GetTexture() : nullptr;
}


ID3D11ShaderResourceView* ModelClassForNoiseFilter::GetTexture3()
{
	return m_Texture3 ? m_Texture3->GetTexture() : nullptr;
}


bool ModelClassForNoiseFilter::LoadModel(HWND hwnd, const char* filename)
{
	if (filename == nullptr)
	{
		return false;
	}
	ifstream fin(filename);
	
	// ������ �� �� ������ �����մϴ�.
	if(fin.fail())
	{
		MessageBox(hwnd, L"Fail to open the model object in Model Class For Noise Filter.", L"Error", MB_OK);
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
	for (int i = 0; i < vertexCount; ++i)
	{
		ModelType& value = model[static_cast<std::size_t>(i)];
		if (!(fin >> value.x >> value.y >> value.z >> value.tu >> value.tv >>
			value.nx >> value.ny >> value.nz))
		{
			return false;
		}
	}

	m_vertexCount = vertexCount;
	m_indexCount = vertexCount;
	m_model = std::move(model);
	return true;
}


void ModelClassForNoiseFilter::ReleaseModel()
{
	m_model.clear();
	m_vertexCount = 0;
	m_indexCount = 0;
}
