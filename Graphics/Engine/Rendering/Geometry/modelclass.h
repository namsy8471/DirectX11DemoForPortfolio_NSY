#ifndef MODELCLASS_H
#define MODELCLASS_H

#include <cstdint>
#include <vector>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "Engine/Scene/CollisionHelpers.h"
#include "Engine/Rendering/Textures/ITexture.h"

// ModelClass is a render resource. Gameplay state and world transforms belong to
// Engine::Scene::GameObject.
class ModelClass final
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
	};

	ModelClass() = default;
	~ModelClass() = default;

	ModelClass(const ModelClass&) = delete;
	ModelClass& operator=(const ModelClass&) = delete;
	ModelClass(ModelClass&&) noexcept = default;
	ModelClass& operator=(ModelClass&&) noexcept = default;

	// HWND is retained only for source compatibility with the legacy call site.
	// ModelClass no longer owns UI or error-reporting behavior.
	[[nodiscard]] bool Initialize(
		HWND window,
		ID3D11Device* device,
		const wchar_t* modelFilename,
		const wchar_t* textureFilename);

	void Shutdown() noexcept;
	void Render(ID3D11DeviceContext* deviceContext) const noexcept;

	[[nodiscard]] int GetIndexCount() const noexcept;
	[[nodiscard]] int GetVertexCount() const noexcept;
	[[nodiscard]] int GetPolygonCount() const noexcept;
	[[nodiscard]] int CountPolygons() const noexcept;
	[[nodiscard]] int CountMeshes() const noexcept;

	[[nodiscard]] ID3D11ShaderResourceView* GetTexture() const noexcept;
	[[nodiscard]] CollisionHelpers::AABB GetLocalAABB() const noexcept;

private:
	[[nodiscard]] bool LoadObj(const wchar_t* filename);
	void GenerateMissingNormals(const std::vector<bool>& needsGeneratedNormal);
	void RecalculateLocalAABB() noexcept;

	[[nodiscard]] bool InitializeBuffers(ID3D11Device* device);
	[[nodiscard]] bool LoadTexture(ID3D11Device* device, const wchar_t* filename) noexcept;

private:
	std::vector<Vertex> m_vertices;
	std::vector<std::uint32_t> m_indices;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Engine::Rendering::TexturePtr m_texture;

	CollisionHelpers::AABB m_localAABB;
};

#endif
