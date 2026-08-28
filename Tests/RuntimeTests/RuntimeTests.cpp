#include "Engine/Core/Error.h"
#include "Engine/Core/Result.h"
#include "Engine/Runtime/FixedStepScheduler.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace
{
	class TestSuite final
	{
	public:
		void Expect(bool condition, const std::string& name)
		{
			if (condition)
			{
				++m_passed;
				std::cout << "[PASS] " << name << '\n';
				return;
			}

			++m_failed;
			std::cerr << "[FAIL] " << name << '\n';
		}

		int ExitCode() const noexcept
		{
			std::cout << "Runtime verification: " << m_passed << " passed, "
				<< m_failed << " failed.\n";
			return m_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
		}

	private:
		int m_passed = 0;
		int m_failed = 0;
	};

	struct DeterministicWorld final
	{
		void FixedUpdate(std::uint64_t tickIndex) noexcept
		{
			m_randomState = m_randomState * 6364136223846793005ull +
				1442695040888963407ull;
			const std::int64_t input = static_cast<std::int64_t>(
				(m_randomState >> 32u) % 3ull) - 1;
			m_position += input * 7 + static_cast<std::int64_t>(tickIndex % 5ull);

			m_hash ^= static_cast<std::uint64_t>(m_position);
			m_hash *= 1099511628211ull;
			m_hash ^= tickIndex;
			m_hash *= 1099511628211ull;
		}

		std::uint64_t Hash() const noexcept
		{
			return m_hash;
		}

	private:
		std::uint64_t m_randomState = 0xC0FFEE1234567890ull;
		std::uint64_t m_hash = 14695981039346656037ull;
		std::int64_t m_position = 0;
	};

	struct ReplayResult final
	{
		std::uint64_t tickCount = 0;
		std::uint64_t worldHash = 0;
	};

	Engine::Result<ReplayResult> RunHeadlessReplay(
		std::uint64_t requestedTicks,
		std::uint32_t ticksPerRenderFrame)
	{
		if (requestedTicks == 0 || ticksPerRenderFrame == 0)
		{
			return Engine::Result<ReplayResult>::Failure(ENGINE_ERROR(
				Engine::ErrorCode::InvalidArgument,
				"RuntimeTests",
				"Replay arguments must be greater than zero."));
		}

		Engine::Runtime::FixedStepScheduler scheduler;
		Engine::Runtime::FixedStepConfig config;
		config.fixedDeltaSeconds = 1.0 / 60.0;
		config.maximumFrameDeltaSeconds = 0.25;
		config.maximumUpdatesPerFrame = 16;
		const Engine::Result<void> configured = scheduler.Configure(config);
		if (!configured)
		{
			return Engine::Result<ReplayResult>::Failure(configured.GetError());
		}

		DeterministicWorld world;
		while (scheduler.CompletedTickCount() < requestedTicks)
		{
			const std::uint64_t remainingTicks =
				requestedTicks - scheduler.CompletedTickCount();
			const auto frameTicks = static_cast<std::uint32_t>(std::min<std::uint64_t>(
				remainingTicks,
				ticksPerRenderFrame));
			const Engine::Runtime::FrameSchedule schedule = scheduler.Advance(
				config.fixedDeltaSeconds * static_cast<double>(frameTicks));

			for (std::uint32_t updateIndex = 0;
				updateIndex < schedule.updateCount;
				++updateIndex)
			{
				world.FixedUpdate(schedule.firstTickIndex + updateIndex);
			}
		}

		ReplayResult result;
		result.tickCount = scheduler.CompletedTickCount();
		result.worldHash = world.Hash();
		return Engine::Result<ReplayResult>::Success(result);
	}

	bool NearlyEqual(double left, double right) noexcept
	{
		return std::abs(left - right) <= 1.0e-9;
	}

	std::string HexHash(std::uint64_t hash)
	{
		std::ostringstream stream;
		stream << "0x" << std::hex << std::uppercase << hash;
		return stream.str();
	}

	std::uint64_t ParseTickCount(int argumentCount, char** arguments)
	{
		if (argumentCount != 3 || std::string(arguments[1]) != "--ticks")
		{
			return 10000;
		}

		try
		{
			const unsigned long long parsed = std::stoull(arguments[2]);
			return parsed > 0 && parsed <= std::numeric_limits<std::uint32_t>::max()
				? static_cast<std::uint64_t>(parsed)
				: 10000;
		}
		catch (...)
		{
			return 10000;
		}
	}
}

int main(int argumentCount, char** arguments)
{
	const std::uint64_t requestedTicks = ParseTickCount(argumentCount, arguments);
	TestSuite tests;

	Engine::Runtime::FixedStepScheduler invalidScheduler;
	Engine::Runtime::FixedStepConfig invalidConfig;
	invalidConfig.fixedDeltaSeconds = 0.0;
	const Engine::Result<void> invalidResult = invalidScheduler.Configure(invalidConfig);
	tests.Expect(!invalidResult, "invalid scheduler configuration is rejected");
	tests.Expect(
		!invalidResult && invalidResult.GetError().Code() == Engine::ErrorCode::InvalidArgument,
		"configuration failure preserves a typed error code");

	Engine::Runtime::FixedStepScheduler stalledScheduler;
	Engine::Runtime::FixedStepConfig stalledConfig;
	stalledConfig.fixedDeltaSeconds = 0.01;
	stalledConfig.maximumFrameDeltaSeconds = 0.05;
	stalledConfig.maximumUpdatesPerFrame = 3;
	const Engine::Result<void> stalledConfigured = stalledScheduler.Configure(stalledConfig);
	tests.Expect(static_cast<bool>(stalledConfigured), "stall scheduler configuration succeeds");
	if (stalledConfigured)
	{
		const Engine::Runtime::FrameSchedule schedule = stalledScheduler.Advance(1.0);
		tests.Expect(schedule.updateCount == 3, "spiral-of-death update budget is enforced");
		tests.Expect(
			NearlyEqual(schedule.acceptedFrameDeltaSeconds, 0.05),
			"long frame delta is clamped");
		tests.Expect(
			NearlyEqual(schedule.droppedTimeSeconds, 0.02),
			"excess simulation backlog is measured");
	}

	const Engine::Result<ReplayResult> oneTickFrames =
		RunHeadlessReplay(requestedTicks, 1);
	const Engine::Result<ReplayResult> groupedFrames =
		RunHeadlessReplay(requestedTicks, 5);
	tests.Expect(
		static_cast<bool>(oneTickFrames) && static_cast<bool>(groupedFrames),
		"headless replay completes for different render-frame partitions");
	if (oneTickFrames && groupedFrames)
	{
		tests.Expect(
			oneTickFrames.Value().tickCount == requestedTicks &&
			groupedFrames.Value().tickCount == requestedTicks,
			"headless replay executes the requested fixed-tick count");
		tests.Expect(
			oneTickFrames.Value().worldHash == groupedFrames.Value().worldHash,
			"world hash is deterministic across render-frame partitions");

		std::cout << "Headless replay: ticks=" << requestedTicks
			<< ", worldHash=" << HexHash(oneTickFrames.Value().worldHash)
			<< '\n';
	}

	return tests.ExitCode();
}
