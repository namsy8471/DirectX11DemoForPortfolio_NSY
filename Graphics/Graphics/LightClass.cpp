#include "stdafx.h"
#include "LightClass.h"


LightClass::LightClass()
{
	m_ambientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_diffuseColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_lookAt = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
}


LightClass::LightClass(const LightClass& other)
{
}


LightClass::~LightClass()
{
}


void LightClass::SetAmbientColor(XMFLOAT4 color)
{
	m_ambientColor = color;
}


void LightClass::SetDiffuseColor(XMFLOAT4 color)
{
	m_diffuseColor = color;
}

void LightClass::SetSpecularColor(XMFLOAT4 color)
{
	m_specularColor = color;
}

void LightClass::SetSpecularPower(float power)
{
	m_specularPower = power;
}

void LightClass::SetPosition(XMFLOAT3 position)
{
	m_position = position;
}

void LightClass::SetLookAt(XMFLOAT3 direction)
{
	m_lookAt = direction;
}


XMFLOAT4 LightClass::GetAmbientColor()
{
	return m_ambientColor;
}


XMFLOAT4 LightClass::GetDiffuseColor()
{
	return m_diffuseColor;
}

XMFLOAT3 LightClass::GetDirection()
{
	return m_lookAt;
}

XMFLOAT4 LightClass::GetSpecularColor()
{
	return m_specularColor;
}

float LightClass::GetSpecularPower()
{
	return m_specularPower;
}

XMFLOAT3 LightClass::GetPosition()
{
	return XMFLOAT3(m_position.x, m_position.y, m_position.z);
}

void LightClass::GenerateViewMatrix()
{
	// ������ ����Ű�� ���͸� �����մϴ�.
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	XMVECTOR upVector = XMLoadFloat3(&up);
	XMVECTOR positionVector = XMLoadFloat3(&m_position);
	XMVECTOR lookAtVector = XMLoadFloat3(&m_lookAt);

	// �� ���ͷκ��� �� ����� ����ϴ�.
	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}


void LightClass::GenerateProjectionMatrix(float screenDepth, float screenNear)
{
	// ���簢�� ������ ���� �þ� �� ȭ�� ������ �����մϴ�.
	float fieldOfView = (float)XM_PI / 2.0f;
	float screenAspect = 1.0f;

	// ���� ���� ����� ����ϴ�.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
}


void LightClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}


void LightClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}