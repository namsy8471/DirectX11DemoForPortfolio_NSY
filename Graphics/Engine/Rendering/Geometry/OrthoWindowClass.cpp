#include "stdafx.h"
#include "Engine/Rendering/Geometry/OrthoWindowClass.h"

#include <cstdint>
#include <utility>
#include <vector>


OrthoWindowClass::OrthoWindowClass()
{
}


OrthoWindowClass::~OrthoWindowClass()
{
	Shutdown();
}


bool OrthoWindowClass::Initialize(ID3D11Device* device, int windowWidth, int windowHeight)
{
	Shutdown();
	if (device == nullptr || windowWidth <= 0 || windowHeight <= 0)
	{
		return false;
	}
	return InitializeBuffers(device, windowWidth, windowHeight);
}


void OrthoWindowClass::Shutdown()
{
	// ���ؽ� �� �ε��� ���۸� �����մϴ�.
	ShutdownBuffers();
}


void OrthoWindowClass::Render(ID3D11DeviceContext* deviceContext)
{
	// �׸��⸦ �غ��ϱ� ���� �׷��� ������ ���ο� �������� �ε��� ���۸� �����ϴ�.
	RenderBuffers(deviceContext);
}


int OrthoWindowClass::GetIndexCount()
{
	return m_indexCount;
}


bool OrthoWindowClass::InitializeBuffers(ID3D11Device* device, int windowWidth, int windowHeight)
{	
	// ������ ������ ȭ�� ��ǥ�� ����մϴ�.
	float left = (float)((windowWidth / 2) * -1);

	// ������ �������� ȭ�� ��ǥ�� ����մϴ�.
	float right = left + (float)windowWidth;

	// ������ ����� ȭ�� ��ǥ�� ����մϴ�.
	float top = (float)(windowHeight / 2);

	// ������ �ϴ��� ȭ�� ��ǥ�� ����մϴ�.
	float bottom = top - (float)windowHeight;

	// ���� �迭�� ���� ���� �����մϴ�.
	m_vertexCount = 6;

	// �ε��� �迭�� �ε��� ���� �����մϴ�.
	m_indexCount = m_vertexCount;

	std::vector<VertexType> vertices(static_cast<std::size_t>(m_vertexCount));
	std::vector<std::uint32_t> indices(static_cast<std::size_t>(m_indexCount));

	// ���� �迭�� �����͸��ε��մϴ�.
	// ù ��° �ﰢ��.
	vertices[0].position = XMFLOAT3(left, top, 0.0f);  // ���� ��
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // ������ �Ʒ�
	vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // ���� �Ʒ�
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

	// �� ��° �ﰢ��.
	vertices[3].position = XMFLOAT3(left, top, 0.0f);  // ���� ��
	vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = XMFLOAT3(right, top, 0.0f);  // ������ ��
	vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // ������ �Ʒ�
	vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

	// �����ͷ� �ε��� �迭���ε��մϴ�.
	for(int i=0; i<m_indexCount; i++)
	{
		indices[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(i);
	}

	// ���� ���� ������ ������ �����Ѵ�.
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

	// ���� ���� ���۸� ����ϴ�.
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer.GetAddressOf())))
	{
		return false;
	}

	// ���� �ε��� ������ ������ �����մϴ�.
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
	if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer.GetAddressOf())))
	{
		return false;
	}

	m_vertexBuffer = std::move(vertexBuffer);
	m_indexBuffer = std::move(indexBuffer);

	return true;
}


void OrthoWindowClass::ShutdownBuffers()
{
	m_indexBuffer.Reset();
	m_vertexBuffer.Reset();
}


void OrthoWindowClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
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
