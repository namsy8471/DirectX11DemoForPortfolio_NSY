#include "Engine/Runtime/FixedStepScheduler.h"

#include "Engine/Core/Error.h"

#include <algorithm>
#include <cmath>

namespace Engine::Runtime
{
	Result<void> FixedStepScheduler::Configure(const FixedStepConfig& config)
	{
		if (!std::isfinite(config.fixedDeltaSeconds) ||
			config.fixedDeltaSeconds <= 0.0)
		{
			return Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::InvalidArgument,
				"FixedStepScheduler",
				"fixedDeltaSeconds must be finite and greater than zero."));
		}

		if (!std::isfinite(config.maximumFrameDeltaSeconds) ||
			config.maximumFrameDeltaSeconds < config.fixedDeltaSeconds)
		{
			return Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::InvalidArgument,
				"FixedStepScheduler",
				"maximumFrameDeltaSeconds must be finite and at least one fixed step."));
		}

		if (config.maximumUpdatesPerFrame == 0)
		{
			return Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::InvalidArgument,
				"FixedStepScheduler",
				"maximumUpdatesPerFrame must be greater than zero."));
		}

		m_config = config;
		Reset();
		return Result<void>::Success();
	}

	FrameSchedule FixedStepScheduler::Advance(double frameDeltaSeconds) noexcept
	{
		FrameSchedule schedule;
		schedule.firstTickIndex = m_completedTickCount;
		schedule.fixedDeltaSeconds = m_config.fixedDeltaSeconds;

		if (!std::isfinite(frameDeltaSeconds) || frameDeltaSeconds < 0.0)
		{
			frameDeltaSeconds = 0.0;
		}

		schedule.acceptedFrameDeltaSeconds = std::min(
			frameDeltaSeconds,
			m_config.maximumFrameDeltaSeconds);
		m_accumulatorSeconds += schedule.acceptedFrameDeltaSeconds;

		const auto availableUpdateCount = static_cast<std::uint64_t>(
			std::floor(m_accumulatorSeconds / m_config.fixedDeltaSeconds));
		schedule.updateCount = static_cast<std::uint32_t>(std::min<std::uint64_t>(
			availableUpdateCount,
			m_config.maximumUpdatesPerFrame));

		m_accumulatorSeconds -=
			static_cast<double>(schedule.updateCount) * m_config.fixedDeltaSeconds;

		if (availableUpdateCount > schedule.updateCount)
		{
			const std::uint64_t droppedUpdateCount =
				availableUpdateCount - schedule.updateCount;
			schedule.droppedTimeSeconds =
				static_cast<double>(droppedUpdateCount) * m_config.fixedDeltaSeconds;
			m_accumulatorSeconds -= schedule.droppedTimeSeconds;
			m_totalDroppedTimeSeconds += schedule.droppedTimeSeconds;
		}

		// Eliminate insignificant negative residue caused by floating-point subtraction.
		m_accumulatorSeconds = std::max(0.0, m_accumulatorSeconds);
		m_completedTickCount += schedule.updateCount;
		schedule.completedTickCount = m_completedTickCount;
		schedule.interpolationAlpha = std::clamp(
			m_accumulatorSeconds / m_config.fixedDeltaSeconds,
			0.0,
			1.0);
		return schedule;
	}

	void FixedStepScheduler::Reset() noexcept
	{
		m_accumulatorSeconds = 0.0;
		m_totalDroppedTimeSeconds = 0.0;
		m_completedTickCount = 0;
	}

	void FixedStepScheduler::DiscardPendingTime() noexcept
	{
		m_accumulatorSeconds = 0.0;
	}

	const FixedStepConfig& FixedStepScheduler::Config() const noexcept
	{
		return m_config;
	}

	std::uint64_t FixedStepScheduler::CompletedTickCount() const noexcept
	{
		return m_completedTickCount;
	}

	double FixedStepScheduler::TotalDroppedTimeSeconds() const noexcept
	{
		return m_totalDroppedTimeSeconds;
	}
}
