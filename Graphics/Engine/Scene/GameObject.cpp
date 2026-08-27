#include "GameObject.h"

#include "../../modelclass.h"

#include <utility>

namespace Engine::Scene
{
	GameObject::~GameObject() = default;

	GameObject::GameObject(std::unique_ptr<ModelClass> model) noexcept
		: m_model(std::move(model))
	{
	}

	bool GameObject::InitializeModel(
		HWND window,
		ID3D11Device* device,
		const wchar_t* modelFilename,
		const wchar_t* textureFilename)
	{
		auto model = std::make_unique<ModelClass>();
		if (!model->Initialize(window, device, modelFilename, textureFilename))
		{
			return false;
		}

		m_model = std::move(model);
		return true;
	}

	void GameObject::SetModel(std::unique_ptr<ModelClass> model) noexcept
	{
		m_model = std::move(model);
	}

	std::unique_ptr<ModelClass> GameObject::ReleaseModel() noexcept
	{
		return std::move(m_model);
	}

	ModelClass* GameObject::GetModel() noexcept
	{
		return m_model.get();
	}

	const ModelClass* GameObject::GetModel() const noexcept
	{
		return m_model.get();
	}

	bool GameObject::HasModel() const noexcept
	{
		return m_model != nullptr;
	}

	void GameObject::SetActive(bool active) noexcept
	{
		m_active = active;
	}

	bool GameObject::IsActive() const noexcept
	{
		return m_active;
	}

	void GameObject::SetTransform(const Transform& transform) noexcept
	{
		m_transform = transform;
	}

	Transform& GameObject::GetTransform() noexcept
	{
		return m_transform;
	}

	const Transform& GameObject::GetTransform() const noexcept
	{
		return m_transform;
	}

	void GameObject::SetPosition(float x, float y, float z) noexcept
	{
		m_transform.SetPosition(x, y, z);
	}

	void GameObject::SetPosition(const DirectX::XMFLOAT3& position) noexcept
	{
		m_transform.SetPosition(position);
	}

	const DirectX::XMFLOAT3& GameObject::GetPosition() const noexcept
	{
		return m_transform.GetPosition();
	}

	void GameObject::GetPosition(float& x, float& y, float& z) const noexcept
	{
		m_transform.GetPosition(x, y, z);
	}

	void GameObject::SetRotationRadians(float x, float y, float z) noexcept
	{
		m_transform.SetRotationRadians(x, y, z);
	}

	void GameObject::SetRotationRadians(const DirectX::XMFLOAT3& rotationRadians) noexcept
	{
		m_transform.SetRotationRadians(rotationRadians);
	}

	void GameObject::SetRotationDegrees(float x, float y, float z) noexcept
	{
		m_transform.SetRotationDegrees(x, y, z);
	}

	const DirectX::XMFLOAT3& GameObject::GetRotationRadians() const noexcept
	{
		return m_transform.GetRotationRadians();
	}

	void GameObject::GetRotationRadians(float& x, float& y, float& z) const noexcept
	{
		m_transform.GetRotationRadians(x, y, z);
	}

	void GameObject::SetScale(float x, float y, float z) noexcept
	{
		m_transform.SetScale(x, y, z);
	}

	void GameObject::SetScale(const DirectX::XMFLOAT3& scale) noexcept
	{
		m_transform.SetScale(scale);
	}

	const DirectX::XMFLOAT3& GameObject::GetScale() const noexcept
	{
		return m_transform.GetScale();
	}

	void GameObject::GetScale(float& x, float& y, float& z) const noexcept
	{
		m_transform.GetScale(x, y, z);
	}

	DirectX::XMMATRIX GameObject::GetWorldMatrix() const noexcept
	{
		return m_transform.GetWorldMatrix();
	}

	CollisionHelpers::AABB GameObject::GetLocalAABB() const noexcept
	{
		return m_model ? m_model->GetLocalAABB() : CollisionHelpers::AABB();
	}

	CollisionHelpers::AABB GameObject::GetWorldAABB() const noexcept
	{
		return m_model
			? m_model->GetLocalAABB().Transform(m_transform.GetWorldMatrix())
			: CollisionHelpers::AABB();
	}

	void GameObject::Render(ID3D11DeviceContext* deviceContext) const noexcept
	{
		if (m_active && m_model)
		{
			m_model->Render(deviceContext);
		}
	}

	int GameObject::GetIndexCount() const noexcept
	{
		return m_model ? m_model->GetIndexCount() : 0;
	}

	int GameObject::GetPolygonCount() const noexcept
	{
		return m_model ? m_model->GetPolygonCount() : 0;
	}

	int GameObject::CountPolygons() const noexcept
	{
		return GetPolygonCount();
	}

	ID3D11ShaderResourceView* GameObject::GetTexture() const noexcept
	{
		return m_model ? m_model->GetTexture() : nullptr;
	}
}
