#include "cameraclass.h"

using namespace DirectX;

CameraClass::CameraClass() noexcept
	: m_viewMatrix(XMMatrixIdentity()),
	  m_baseViewMatrix(XMMatrixIdentity())
{
}

bool CameraClass::Initialize() noexcept
{
	m_pitchRadians = XMConvertToRadians(m_rotationDegrees.x);
	m_yawRadians = XMConvertToRadians(m_rotationDegrees.y);
	m_rollRadians = XMConvertToRadians(m_rotationDegrees.z);
	UpdateViewMatrix();
	CaptureBaseViewMatrix();
	return true;
}

void CameraClass::SetPosition(float x, float y, float z) noexcept
{
	m_position = XMFLOAT3(x, y, z);
}

void CameraClass::SetPositionY(float y) noexcept
{
	m_position.y = y;
}

void CameraClass::SetRotation(
	float pitchDegrees,
	float yawDegrees,
	float rollDegrees) noexcept
{
	m_rotationDegrees = XMFLOAT3(pitchDegrees, yawDegrees, rollDegrees);
	m_pitchRadians = XMConvertToRadians(pitchDegrees);
	m_yawRadians = XMConvertToRadians(yawDegrees);
	m_rollRadians = XMConvertToRadians(rollDegrees);
}

void CameraClass::SetCamYaw(float yawRadians) noexcept
{
	m_yawRadians = yawRadians;
	m_rotationDegrees.y = XMConvertToDegrees(yawRadians);
}

void CameraClass::SetCamPitch(float pitchRadians) noexcept
{
	m_pitchRadians = pitchRadians;
	m_rotationDegrees.x = XMConvertToDegrees(pitchRadians);
}

XMFLOAT3 CameraClass::GetPosition() const noexcept
{
	return m_position;
}

XMFLOAT3 CameraClass::GetRotation() const noexcept
{
	return m_rotationDegrees;
}

float CameraClass::GetCamYaw() const noexcept
{
	return m_yawRadians;
}

float CameraClass::GetCamPitch() const noexcept
{
	return m_pitchRadians;
}

void CameraClass::MoveLocal(float moveRight, float moveForward) noexcept
{
	const XMMATRIX yawRotation = XMMatrixRotationY(m_yawRadians);
	const XMVECTOR right = XMVector3TransformNormal(
		XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),
		yawRotation);
	const XMVECTOR forward = XMVector3TransformNormal(
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		yawRotation);

	XMVECTOR position = XMLoadFloat3(&m_position);
	position += moveRight * right;
	position += moveForward * forward;
	XMStoreFloat3(&m_position, position);
}

void CameraClass::UpdateViewMatrix() noexcept
{
	const XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
		m_pitchRadians,
		m_yawRadians,
		m_rollRadians);
	const XMVECTOR forward = XMVector3Normalize(XMVector3TransformNormal(
		XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		rotation));
	const XMVECTOR up = XMVector3Normalize(XMVector3TransformNormal(
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
		rotation));
	const XMVECTOR position = XMLoadFloat3(&m_position);
	m_viewMatrix = XMMatrixLookAtLH(position, position + forward, up);
}

void CameraClass::CaptureBaseViewMatrix() noexcept
{
	m_baseViewMatrix = m_viewMatrix;
}

void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix) const noexcept
{
	viewMatrix = m_viewMatrix;
}

void CameraClass::GetBaseViewMatrix(XMMATRIX& viewMatrix) const noexcept
{
	viewMatrix = m_baseViewMatrix;
}
