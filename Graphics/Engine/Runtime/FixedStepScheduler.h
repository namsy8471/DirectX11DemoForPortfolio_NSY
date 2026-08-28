#pragma once

#include "Engine/Core/Result.h"

#include <cstdint>

namespace Engine::Runtime
{
	struct FixedStepConfig final
	{
		double fixedDeltaSeconds = 1.0 / 60.0;
		double maximumFrameDeltaSeconds = 0.1;
		std::uint32_t maximumUpdatesPerFrame = 8;
	};

	struct FrameSchedule final
	{
		std::uint64_t firstTickIndex = 0;
		std::uint64_t completedTickCount = 0;
		std::uint32_t updateCount = 0;
		double fixedDeltaSeconds = 0.0;
		double acceptedFrameDeltaSeconds = 0.0;
		double interpolationAlpha = 0.0;
		double droppedTimeSeconds = 0.0;
	};

	class FixedStepScheduler final
	{
	public:
		Result<void> Configure(const FixedStepConfig& config);
		FrameSchedule Advance(double frameDeltaSeconds) noexcept;

		void Reset() noexcept;
		void DiscardPendingTime() noexcept;

		const FixedStepConfig& Config() const noexcept;
		std::uint64_t CompletedTickCount() const noexcept;
		double TotalDroppedTimeSeconds() const noexcept;

	private:
		FixedStepConfig m_config;
		double m_accumulatorSeconds = 0.0;
		double m_totalDroppedTimeSeconds = 0.0;
		std::uint64_t m_completedTickCount = 0;
	};
}
