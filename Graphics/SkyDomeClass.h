#pragma once


#include <cstdint>
#include <vector>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using namespace DirectX;

class SkyDomeClass
{
private:
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VertexType
	{
		XMFLOAT3 position;
	};

public:
	SkyDomeClass();
	SkyDomeClass(const SkyDomeClass&) = delete;
	SkyDomeClass& operator=(const SkyDomeClass&) = delete;
	~SkyDomeClass();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	XMFLOAT4 GetApexColor();
	XMFLOAT4 GetCenterColor();

private:
	bool LoadSkyDomeModel(const char*);
	void ReleaseSkyDomeModel();

	bool InitializeBuffers(ID3D11Device*);
	void ReleaseBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	std::vector<ModelType> m_model;
	int m_vertexCount = 0;
	int m_indexCount = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	XMFLOAT4 m_apexColor;
	XMFLOAT4 m_centerColor;
};
