#pragma once

#include <cstdint>

namespace Engine
{
	// Deterministic simulation data. Every update receives the same delta and is
	// identified by a monotonically increasing tick index.
	struct FixedFrameContext final
	{
		std::uint64_t tickIndex = 0;
		float deltaMilliseconds = 0.0f;
		float fixedDeltaSeconds = 0.0f;
		double simulationTimeSeconds = 0.0;
		int fps = 0;
		int cpuPercentage = 0;
	};

	// Presentation data for one rendered frame. interpolationAlpha describes how
	// far real time has advanced between the two most recent simulation states.
	struct RenderFrameContext final
	{
		std::uint64_t frameIndex = 0;
		std::uint64_t completedFixedTickCount = 0;
		float frameDeltaMilliseconds = 0.0f;
		float frameDeltaSeconds = 0.0f;
		double elapsedRealTimeSeconds = 0.0;
		float interpolationAlpha = 0.0f;
		int fps = 0;
		int cpuPercentage = 0;
	};
}
