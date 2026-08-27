#include "FirstPersonCameraController.h"
#include "InputSnapshot.h"

#include "../../cameraclass.h"

#include <algorithm>

namespace
{
	constexpr float PitchLimit = DirectX::XM_PIDIV2 - 0.01f;
}

namespace Engine
{
	FirstPersonCameraController::FirstPersonCameraController(
		float movementUnitsPerMillisecond,
		float lookRadiansPerMouseUnit) noexcept
		: m_movementUnitsPerMillisecond((std::max)(movementUnitsPerMillisecond, 0.0f)),
		  m_lookRadiansPerMouseUnit((std::max)(lookRadiansPerMouseUnit, 0.0f))
	{
	}

	void FirstPersonCameraController::Update(
		CameraClass& camera,
		const Input::InputSnapshot& input,
		float deltaMilliseconds) const noexcept
	{
		const float safeDeltaMilliseconds = (std::max)(deltaMilliseconds, 0.0f);
		const float yaw = camera.GetCamYaw() +
			(input.lookX * m_lookRadiansPerMouseUnit);
		const float pitch = std::clamp(
			camera.GetCamPitch() + (input.lookY * m_lookRadiansPerMouseUnit),
			-PitchLimit,
			PitchLimit);

		camera.SetCamYaw(yaw);
		camera.SetCamPitch(pitch);

		const float moveRight = input.moveRight *
			m_movementUnitsPerMillisecond * safeDeltaMilliseconds;
		const float moveForward = input.moveForward *
			m_movementUnitsPerMillisecond * safeDeltaMilliseconds;
		camera.MoveLocal(moveRight, moveForward);
		camera.UpdateViewMatrix();
	}

	float FirstPersonCameraController::GetMovementUnitsPerMillisecond() const noexcept
	{
		return m_movementUnitsPerMillisecond;
	}

	float FirstPersonCameraController::GetLookRadiansPerMouseUnit() const noexcept
	{
		return m_lookRadiansPerMouseUnit;
	}
}
