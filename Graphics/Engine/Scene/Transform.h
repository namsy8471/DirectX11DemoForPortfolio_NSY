#pragma once

#include <DirectXMath.h>

namespace Engine::Scene
{
	class Transform final
	{
	public:
		Transform() = default;

		void SetPosition(float x, float y, float z) noexcept
		{
			m_position = DirectX::XMFLOAT3(x, y, z);
		}

		void SetPosition(const DirectX::XMFLOAT3& position) noexcept
		{
			m_position = position;
		}

		[[nodiscard]] const DirectX::XMFLOAT3& GetPosition() const noexcept
		{
			return m_position;
		}

		void GetPosition(float& x, float& y, float& z) const noexcept
		{
			x = m_position.x;
			y = m_position.y;
			z = m_position.z;
		}

		void SetRotationRadians(float x, float y, float z) noexcept
		{
			m_rotationRadians = DirectX::XMFLOAT3(x, y, z);
		}

		void SetRotationRadians(const DirectX::XMFLOAT3& rotationRadians) noexcept
		{
			m_rotationRadians = rotationRadians;
		}

		void SetRotationDegrees(float x, float y, float z) noexcept
		{
			SetRotationRadians(
				DirectX::XMConvertToRadians(x),
				DirectX::XMConvertToRadians(y),
				DirectX::XMConvertToRadians(z));
		}

		[[nodiscard]] const DirectX::XMFLOAT3& GetRotationRadians() const noexcept
		{
			return m_rotationRadians;
		}

		void GetRotationRadians(float& x, float& y, float& z) const noexcept
		{
			x = m_rotationRadians.x;
			y = m_rotationRadians.y;
			z = m_rotationRadians.z;
		}

		void SetScale(float x, float y, float z) noexcept
		{
			m_scale = DirectX::XMFLOAT3(x, y, z);
		}

		void SetScale(const DirectX::XMFLOAT3& scale) noexcept
		{
			m_scale = scale;
		}

		[[nodiscard]] const DirectX::XMFLOAT3& GetScale() const noexcept
		{
			return m_scale;
		}

		void GetScale(float& x, float& y, float& z) const noexcept
		{
			x = m_scale.x;
			y = m_scale.y;
			z = m_scale.z;
		}

		[[nodiscard]] DirectX::XMMATRIX GetWorldMatrix() const noexcept
		{
			const DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(
				m_scale.x,
				m_scale.y,
				m_scale.z);
			const DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(
				m_rotationRadians.x,
				m_rotationRadians.y,
				m_rotationRadians.z);
			const DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(
				m_position.x,
				m_position.y,
				m_position.z);
			return scale * rotation * translation;
		}

	private:
		DirectX::XMFLOAT3 m_position{0.0f, 0.0f, 0.0f};
		DirectX::XMFLOAT3 m_rotationRadians{0.0f, 0.0f, 0.0f};
		DirectX::XMFLOAT3 m_scale{1.0f, 1.0f, 1.0f};
	};
}
