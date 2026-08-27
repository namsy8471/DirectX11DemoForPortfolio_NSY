#include "LightClass.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

LightClass::LightClass() noexcept
	: m_viewMatrix(XMMatrixIdentity()),
	  m_projectionMatrix(XMMatrixIdentity())
{
}

void LightClass::SetAmbientColor(const XMFLOAT4& color) noexcept
{
	m_ambientColor = color;
}

void LightClass::SetDiffuseColor(const XMFLOAT4& color) noexcept
{
	m_diffuseColor = color;
}

void LightClass::SetPosition(const XMFLOAT3& position) noexcept
{
	m_position = position;
}

void LightClass::SetLookAt(const XMFLOAT3& lookAt) noexcept
{
	m_lookAt = lookAt;
}

void LightClass::SetSpecularColor(const XMFLOAT4& color) noexcept
{
	m_specularColor = color;
}

void LightClass::SetSpecularPower(float power) noexcept
{
	m_specularPower = (std::max)(power, 0.0f);
}

XMFLOAT4 LightClass::GetAmbientColor() const noexcept
{
	return m_ambientColor;
}

XMFLOAT4 LightClass::GetDiffuseColor() const noexcept
{
	return m_diffuseColor;
}

XMFLOAT3 LightClass::GetDirection() const noexcept
{
	const XMVECTOR position = XMLoadFloat3(&m_position);
	const XMVECTOR toTarget = XMLoadFloat3(&m_lookAt) - position;
	if (XMVectorGetX(XMVector3LengthSq(toTarget)) <= 1.0e-12f)
	{
		return XMFLOAT3{0.0f, 0.0f, 1.0f};
	}

	XMFLOAT3 direction;
	XMStoreFloat3(&direction, XMVector3Normalize(toTarget));
	return direction;
}

XMFLOAT4 LightClass::GetSpecularColor() const noexcept
{
	return m_specularColor;
}

float LightClass::GetSpecularPower() const noexcept
{
	return m_specularPower;
}

XMFLOAT3 LightClass::GetPosition() const noexcept
{
	return m_position;
}

void LightClass::GenerateViewMatrix() noexcept
{
	const XMVECTOR position = XMLoadFloat3(&m_position);
	XMVECTOR direction = XMLoadFloat3(&m_lookAt) - position;
	if (XMVectorGetX(XMVector3LengthSq(direction)) <= 1.0e-12f)
	{
		direction = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}
	direction = XMVector3Normalize(direction);

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	if (std::abs(XMVectorGetX(XMVector3Dot(direction, up))) > 0.999f)
	{
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	m_viewMatrix = XMMatrixLookAtLH(
		position,
		position + direction,
		up);
}

void LightClass::GenerateProjectionMatrix(float screenDepth, float screenNear) noexcept
{
	const float safeNear = (std::max)(screenNear, 0.001f);
	const float safeDepth = (std::max)(screenDepth, safeNear + 0.001f);
	m_projectionMatrix = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		1.0f,
		safeNear,
		safeDepth);
}

void LightClass::GetViewMatrix(XMMATRIX& viewMatrix) const noexcept
{
	viewMatrix = m_viewMatrix;
}

void LightClass::GetProjectionMatrix(XMMATRIX& projectionMatrix) const noexcept
{
	projectionMatrix = m_projectionMatrix;
}
