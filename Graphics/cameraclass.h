#pragma once

#include <DirectXMath.h>

#include "AlignedAllocationPolicy.h"

// CameraClass owns only camera state and view matrices. Input mapping lives in
// FirstPersonCameraController, while rendering consumes the matrices as data.
class CameraClass final : public AlignedAllocationPolicy<16>
{
public:
	CameraClass() noexcept;
	~CameraClass() = default;

	CameraClass(const CameraClass&) = delete;
	CameraClass& operator=(const CameraClass&) = delete;

	[[nodiscard]] bool Initialize() noexcept;

	void SetPosition(float x, float y, float z) noexcept;
	void SetPositionY(float y) noexcept;
	void SetRotation(float pitchDegrees, float yawDegrees, float rollDegrees) noexcept;
	void SetCamYaw(float yawRadians) noexcept;
	void SetCamPitch(float pitchRadians) noexcept;

	[[nodiscard]] DirectX::XMFLOAT3 GetPosition() const noexcept;
	[[nodiscard]] DirectX::XMFLOAT3 GetRotation() const noexcept;
	[[nodiscard]] float GetCamYaw() const noexcept;
	[[nodiscard]] float GetCamPitch() const noexcept;

	// Applies movement immediately during Update, so collision and terrain code
	// observe the same camera state that the subsequent Render consumes.
	void MoveLocal(float moveRight, float moveForward) noexcept;
	void UpdateViewMatrix() noexcept;
	void CaptureBaseViewMatrix() noexcept;

	void GetViewMatrix(DirectX::XMMATRIX& viewMatrix) const noexcept;
	void GetBaseViewMatrix(DirectX::XMMATRIX& viewMatrix) const noexcept;

private:
	DirectX::XMFLOAT3 m_position{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_rotationDegrees{0.0f, 0.0f, 0.0f};
	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_baseViewMatrix;
	float m_yawRadians = 0.0f;
	float m_pitchRadians = 0.0f;
	float m_rollRadians = 0.0f;
};
