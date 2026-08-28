#pragma once



/////////////
// GLOBALS //
/////////////
const int TEXTURE_REPEAT = 8;

#include <memory>
#include <vector>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "Engine/Rendering/Textures/ITexture.h"

using namespace DirectX;

class TerrainClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	    XMFLOAT3 normal;
	};

	struct HeightMapType 
	{ 
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VectorType 
	{ 
		float x, y, z;
	};

public:
	TerrainClass();
	TerrainClass(const TerrainClass&) = delete;
	TerrainClass& operator=(const TerrainClass&) = delete;
	~TerrainClass();

	bool Initialize(ID3D11Device*, const char*, const WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
	void SetWorldOffset(const XMFLOAT3& worldOffset) noexcept;
	
	[[nodiscard]] float GetHeight(float x, float z) const noexcept;

private:
	bool LoadHeightMap(const char*);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();

	void CalculateTextureCoordinates();
	bool LoadTexture(ID3D11Device*, const WCHAR*);
	void ReleaseTexture();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	
private:
	int m_terrainWidth = 0;
	int m_terrainHeight = 0;
	int m_vertexCount = 0;
	int m_indexCount = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	std::vector<HeightMapType> m_heightMap;
	Engine::Rendering::TexturePtr m_texture;
	XMFLOAT3 m_worldOffset{0.0f, 0.0f, 0.0f};
};
