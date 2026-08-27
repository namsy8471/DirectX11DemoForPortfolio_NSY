#pragma once

#include <cstdint>

namespace Engine
{
	// Immutable-by-convention data shared by every update and render in a frame.
	// The legacy TimerClass reports milliseconds, so both units are exposed
	// explicitly to prevent accidental unit conversions in game code.
	struct FrameContext final
	{
		std::uint64_t frameIndex = 0;
		float deltaMilliseconds = 0.0f;
		float deltaTimeSeconds = 0.0f;
		double elapsedTimeSeconds = 0.0;
		int fps = 0;
		int cpuPercentage = 0;
	};
}
