#pragma once

#include <DirectXMath.h>

class LightClass final
{
public:
	LightClass() noexcept;
	~LightClass() = default;

	LightClass(const LightClass&) = delete;
	LightClass& operator=(const LightClass&) = delete;

	void SetAmbientColor(const DirectX::XMFLOAT4& color) noexcept;
	void SetDiffuseColor(const DirectX::XMFLOAT4& color) noexcept;
	void SetPosition(const DirectX::XMFLOAT3& position) noexcept;
	void SetLookAt(const DirectX::XMFLOAT3& lookAt) noexcept;
	void SetSpecularColor(const DirectX::XMFLOAT4& color) noexcept;
	void SetSpecularPower(float power) noexcept;

	[[nodiscard]] DirectX::XMFLOAT4 GetAmbientColor() const noexcept;
	[[nodiscard]] DirectX::XMFLOAT4 GetDiffuseColor() const noexcept;
	[[nodiscard]] DirectX::XMFLOAT3 GetDirection() const noexcept;
	[[nodiscard]] DirectX::XMFLOAT4 GetSpecularColor() const noexcept;
	[[nodiscard]] float GetSpecularPower() const noexcept;
	[[nodiscard]] DirectX::XMFLOAT3 GetPosition() const noexcept;

	void GenerateViewMatrix() noexcept;
	void GenerateProjectionMatrix(float screenDepth, float screenNear) noexcept;
	void GetViewMatrix(DirectX::XMMATRIX& viewMatrix) const noexcept;
	void GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix) const noexcept;

private:
	DirectX::XMFLOAT4 m_ambientColor{0.0f, 0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT4 m_diffuseColor{0.0f, 0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_lookAt{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT4 m_specularColor{0.0f, 0.0f, 0.0f, 0.0f};
	float m_specularPower = 0.0f;
	DirectX::XMFLOAT3 m_position{0.0f, 0.0f, 0.0f};
	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_projectionMatrix;
};
