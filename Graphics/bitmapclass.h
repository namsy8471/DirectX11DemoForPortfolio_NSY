////////////////////////////////////////////////////////////////////////////////
// Filename: bitmapclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _BITMAPCLASS_H_
#define _BITMAPCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <memory>
#include <wrl/client.h>

using namespace DirectX;


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: BitmapClass
////////////////////////////////////////////////////////////////////////////////
class BitmapClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:
	BitmapClass();
	BitmapClass(const BitmapClass&) = delete;
	BitmapClass& operator=(const BitmapClass&) = delete;
	~BitmapClass();

	bool Initialize(ID3D11Device*, int, int, const WCHAR*, int, int);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, int);

	void SetPosition(float, float, float);
	void SetScale(float, float, float);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
	void GetPosition(float&, float&, float&);
	void GetScale(float&, float&, float&);

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, const WCHAR*);
	void ReleaseTexture();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	int m_vertexCount = 0;
	int m_indexCount = 0;
	std::unique_ptr<TextureClass> m_Texture;
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	int m_bitmapWidth = 0;
	int m_bitmapHeight = 0;
	int m_previousPosX = -1;
	int m_previousPosY = -1;
	XMFLOAT3 m_position{0.0f, 0.0f, 0.0f};
	XMFLOAT3 m_scale{1.0f, 1.0f, 1.0f};
};

#endif
