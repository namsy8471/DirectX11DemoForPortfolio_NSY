#pragma once

class CameraClass;

namespace Engine::Input
{
	struct InputSnapshot;
}

namespace Engine
{
	class FirstPersonCameraController final
	{
	public:
		explicit FirstPersonCameraController(
			float movementUnitsPerMillisecond = 0.012f,
			float lookRadiansPerMouseUnit = 0.001f) noexcept;

		void Update(
			CameraClass& camera,
			const Input::InputSnapshot& input,
			float deltaMilliseconds) const noexcept;

		[[nodiscard]] float GetMovementUnitsPerMillisecond() const noexcept;
		[[nodiscard]] float GetLookRadiansPerMouseUnit() const noexcept;

	private:
		float m_movementUnitsPerMillisecond;
		float m_lookRadiansPerMouseUnit;
	};
}
