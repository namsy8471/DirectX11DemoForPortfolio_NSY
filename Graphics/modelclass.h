////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

/////////////
// LINKING //
/////////////
#pragma comment(lib, "lib/assimp-vc142-mtd.lib")

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

#include <fstream>
#include <atlstr.h>
#include "textureclass.h"
#include "CollisionHelpers.h"

using namespace std;

// ASSIMP library
#include "include/assimp/Importer.hpp"
#include "include/assimp/scene.h"
#include "include/assimp/postprocess.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
private:
	struct VertexTypeForAssimp
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(HWND, ID3D11Device*, const WCHAR*, const WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();
	ID3D11ShaderResourceView* GetTexture2();
	ID3D11ShaderResourceView* GetTexture3();

	void SetPosition(float, float, float);
	void GetPosition(float&, float&, float&);

	void SetRotationByDegrees(float, float, float);
	void GetRotation(float&, float&, float&);

	void SetScale(float, float, float);
	void GetScale(float&, float&, float&);

	int CountPolygons();
	int CountMeshes();

	bool isAlive() { return alive; }
	void SetDie() { alive = false; }

	// AABB °ü·Ã ¸Þ¼­µå
	CollisionHelpers::AABB GetLocalAABB() const { return m_localAABB; }
	CollisionHelpers::AABB GetWorldAABB() const;
	XMMATRIX GetWorldMatrix() const;

private:
	bool InitializeBuffers(HWND, ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, const WCHAR*);
	void ReleaseTexture();

	bool LoadModel(CString, UINT flag);
	void ReleaseModel();

private:

	bool alive = true;

	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	unsigned int m_vertexCount, m_indexCount, m_faceCount;
	
	VertexTypeForAssimp* m_vertices;
	unsigned long* m_indices;

	TextureClass* m_Texture;

	XMFLOAT3 m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	// Local space AABB (¸ðµ¨ ÁÂÇ¥°è)
	CollisionHelpers::AABB m_localAABB;
};

#endif