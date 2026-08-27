#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "textureclass.h"

using namespace DirectX;

class ModelClassForNoiseFilter
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	ModelClassForNoiseFilter();
	ModelClassForNoiseFilter(const ModelClassForNoiseFilter&) = delete;
	ModelClassForNoiseFilter& operator=(const ModelClassForNoiseFilter&) = delete;
	~ModelClassForNoiseFilter();

	bool Initialize(HWND, ID3D11Device*, const char*, const WCHAR*, const WCHAR*, const WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture1();
	ID3D11ShaderResourceView* GetTexture2();
	ID3D11ShaderResourceView* GetTexture3();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTextures(ID3D11Device*, const WCHAR*, const WCHAR*, const WCHAR*);
	void ReleaseTextures();

	bool LoadModel(HWND, const char*);
	void ReleaseModel();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	int m_vertexCount = 0;
	int m_indexCount = 0;
	std::unique_ptr<TextureClass> m_Texture1;
	std::unique_ptr<TextureClass> m_Texture2;
	std::unique_ptr<TextureClass> m_Texture3;
	std::vector<ModelType> m_model;
};
