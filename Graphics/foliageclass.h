#pragma once


#include <memory>
#include <vector>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using namespace DirectX;

class TextureClass;
class TerrainClass;  // ���� ���� �߰�


class FoliageClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

	struct FoliageType
	{
		float x, z;
		float r, g, b;
	};

	struct InstanceType
	{
		XMMATRIX matrix;
		XMFLOAT3 color;
	};

public:
	FoliageClass();
	FoliageClass(const FoliageClass&) = delete;
	FoliageClass& operator=(const FoliageClass&) = delete;
	~FoliageClass();

	bool Initialize(ID3D11Device*, const WCHAR*, int);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool Frame(XMFLOAT3, ID3D11DeviceContext*, float, TerrainClass*);  // ���� �Ķ���� �߰�

	int GetVertexCount();
	int GetInstanceCount();

	void SetPosition(float x, float y, float z);
	void GetPosition(float& x, float& y, float& z);

	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, const WCHAR*);
	void ReleaseTexture();

	bool GeneratePositions();

private:
	int m_foliageCount = 0;

	XMFLOAT3 m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);

	std::vector<FoliageType> m_foliageArray;
	std::vector<InstanceType> m_Instances;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceBuffer;
	int m_vertexCount = 0;
	int m_instanceCount = 0;
	std::unique_ptr<TextureClass> m_Texture;
	float m_windRotation = 0.0f;
	int m_windDirection = 0;
};
