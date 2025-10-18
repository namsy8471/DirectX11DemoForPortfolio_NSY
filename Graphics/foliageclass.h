#pragma once


class TextureClass;
class TerrainClass;  // 전방 선언 추가


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
	FoliageClass(const FoliageClass&);
	~FoliageClass();

	bool Initialize(ID3D11Device*, const WCHAR*, int);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool Frame(XMFLOAT3, ID3D11DeviceContext*, float, TerrainClass*);  // 지형 파라미터 추가

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

	FoliageType* m_foliageArray = nullptr;
	InstanceType* m_Instances = nullptr;
	ID3D11Buffer *m_vertexBuffer = nullptr;
	ID3D11Buffer *m_instanceBuffer = nullptr;
	int m_vertexCount = 0;
	int m_instanceCount = 0;
	TextureClass* m_Texture = nullptr;
	float m_windRotation = 0.0f;
	int m_windDirection = 0;
};