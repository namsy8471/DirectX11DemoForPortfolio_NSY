#pragma once

#include <d3d11.h>
#include <memory>

#include "Engine/Scene/Transform.h"
#include "Engine/Scene/CollisionHelpers.h"

class ModelClass;

namespace Engine::Scene
{
	class GameObject final
	{
	public:
		GameObject() = default;
		explicit GameObject(std::unique_ptr<ModelClass> model) noexcept;
		~GameObject();

		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		GameObject(GameObject&&) noexcept = default;
		GameObject& operator=(GameObject&&) noexcept = default;

		[[nodiscard]] bool InitializeModel(
			HWND window,
			ID3D11Device* device,
			const wchar_t* modelFilename,
			const wchar_t* textureFilename);

		void SetModel(std::unique_ptr<ModelClass> model) noexcept;
		[[nodiscard]] std::unique_ptr<ModelClass> ReleaseModel() noexcept;
		[[nodiscard]] ModelClass* GetModel() noexcept;
		[[nodiscard]] const ModelClass* GetModel() const noexcept;
		[[nodiscard]] bool HasModel() const noexcept;

		void SetActive(bool active) noexcept;
		[[nodiscard]] bool IsActive() const noexcept;

		void SetTransform(const Transform& transform) noexcept;
		[[nodiscard]] Transform& GetTransform() noexcept;
		[[nodiscard]] const Transform& GetTransform() const noexcept;

		void SetPosition(float x, float y, float z) noexcept;
		void SetPosition(const DirectX::XMFLOAT3& position) noexcept;
		[[nodiscard]] const DirectX::XMFLOAT3& GetPosition() const noexcept;
		void GetPosition(float& x, float& y, float& z) const noexcept;

		void SetRotationRadians(float x, float y, float z) noexcept;
		void SetRotationRadians(const DirectX::XMFLOAT3& rotationRadians) noexcept;
		void SetRotationDegrees(float x, float y, float z) noexcept;
		[[nodiscard]] const DirectX::XMFLOAT3& GetRotationRadians() const noexcept;
		void GetRotationRadians(float& x, float& y, float& z) const noexcept;

		void SetScale(float x, float y, float z) noexcept;
		void SetScale(const DirectX::XMFLOAT3& scale) noexcept;
		[[nodiscard]] const DirectX::XMFLOAT3& GetScale() const noexcept;
		void GetScale(float& x, float& y, float& z) const noexcept;

		[[nodiscard]] DirectX::XMMATRIX GetWorldMatrix() const noexcept;
		[[nodiscard]] CollisionHelpers::AABB GetLocalAABB() const noexcept;
		[[nodiscard]] CollisionHelpers::AABB GetWorldAABB() const noexcept;

		void Render(ID3D11DeviceContext* deviceContext) const noexcept;
		[[nodiscard]] int GetIndexCount() const noexcept;
		[[nodiscard]] int GetPolygonCount() const noexcept;
		[[nodiscard]] int CountPolygons() const noexcept;
		[[nodiscard]] ID3D11ShaderResourceView* GetTexture() const noexcept;

	private:
		std::unique_ptr<ModelClass> m_model;
		Transform m_transform;
		bool m_active = true;
	};
}
