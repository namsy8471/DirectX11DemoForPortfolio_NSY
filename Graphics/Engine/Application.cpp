#include "Engine/Application.h"

#include "Engine/Core/Error.h"
#include "Engine/Core/Log.h"

#include <cstdlib>
#include <exception>
#include <optional>
#include <string>
#include <utility>

namespace Engine
{
	Application::Application(
		std::unique_ptr<IGame> game,
		WindowConfig windowConfig,
		Runtime::FixedStepConfig fixedStepConfig)
		: m_windowConfig(std::move(windowConfig)),
		  m_fixedStepConfig(fixedStepConfig),
		  m_game(std::move(game))
	{
	}

	Application::~Application()
	{
		Shutdown();
	}

	Result<void> Application::Initialize()
	{
		if (m_lifecycle == Lifecycle::Initialized ||
			m_lifecycle == Lifecycle::Running)
		{
			return Result<void>::Success();
		}

		if (m_lifecycle != Lifecycle::Constructed || !m_game)
		{
			return Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::InvariantViolation,
				"Application",
				"Initialize was called from an invalid lifecycle state or without a game."));
		}

		m_lifecycle = Lifecycle::Initializing;
		const Result<void> schedulerConfigured =
			m_fixedStepScheduler.Configure(m_fixedStepConfig);
		if (!schedulerConfigured)
		{
			m_lifecycle = Lifecycle::Stopped;
			return Result<void>::Failure(schedulerConfigured.GetError());
		}

		if (!m_window.Create(m_windowConfig, {}))
		{
			m_lifecycle = Lifecycle::Stopped;
			return Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::PlatformFailure,
				"Win32Window",
				"Could not create the native application window."));
		}

		// Shutdown is required even when Initialize returns false because a game
		// may have acquired only a prefix of its resources.
		m_gameNeedsShutdown = true;
		Result<void> gameInitialized = Result<void>::Failure(ENGINE_ERROR(
			ErrorCode::InitializationFailed,
			"Game",
			"Game initialization did not complete."));
		try
		{
			gameInitialized = m_game->Initialize(m_window.GetNativeWindow());
		}
		catch (const std::exception& exception)
		{
			gameInitialized = Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::UnexpectedFailure,
				"Game",
				std::string("Game initialization threw an exception: ") + exception.what()));
		}
		catch (...)
		{
			gameInitialized = Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::UnexpectedFailure,
				"Game",
				"Game initialization threw an unknown exception."));
		}

		if (!gameInitialized)
		{
			const Error error = gameInitialized.GetError();
			Shutdown();
			return Result<void>::Failure(error);
		}

		// Start telemetry only after resource loading. Otherwise the first frame
		// would include the entire initialization duration in its delta time.
		if (!m_timer.Initialize())
		{
			Shutdown();
			return Result<void>::Failure(ENGINE_ERROR(
				ErrorCode::InitializationFailed,
				"Timer",
				"Could not initialize the runtime timer."));
		}
		m_fps.Initialize();
		// CPU telemetry is optional; the game loop remains usable when PDH is
		// unavailable on the host.
		m_cpuNeedsShutdown = m_cpu.Initialize();

		m_frameIndex = 0;
		m_elapsedRealTimeSeconds = 0.0;
		m_lifecycle = Lifecycle::Initialized;
		Log::Write(LogLevel::Info, "Application", "Runtime initialized successfully.");
		return Result<void>::Success();
	}

	Result<int> Application::Run()
	{
		if (m_lifecycle == Lifecycle::Constructed)
		{
			const Result<void> initialized = Initialize();
			if (!initialized)
			{
				return Result<int>::Failure(initialized.GetError());
			}
		}

		if (m_lifecycle != Lifecycle::Initialized)
		{
			return Result<int>::Failure(ENGINE_ERROR(
				ErrorCode::InvariantViolation,
				"Application",
				"Run was called while the application was not initialized."));
		}

		m_lifecycle = Lifecycle::Running;
		int exitCode = EXIT_SUCCESS;
		bool wasSuspended = false;
		bool exitRequested = false;
		std::optional<Error> runtimeError;
		m_timer.Reset();
		m_fps.Initialize();
		m_fixedStepScheduler.Reset();
		Log::Write(LogLevel::Info, "Application", "Entering the fixed-step runtime loop.");

		while (m_window.PumpMessages(exitCode))
		{
			if (m_window.IsMinimized())
			{
				wasSuspended = true;
				m_timer.Reset();
				m_window.WaitForActivity();
				continue;
			}
			if (wasSuspended)
			{
				m_timer.Reset();
				m_fps.Initialize();
				m_fixedStepScheduler.DiscardPendingTime();
				wasSuspended = false;
			}

			m_timer.Frame();
			m_fps.Frame();
			m_cpu.Frame();

			const double measuredFrameDeltaSeconds =
				static_cast<double>(m_timer.GetTime()) * 0.001;
			const Runtime::FrameSchedule schedule =
				m_fixedStepScheduler.Advance(measuredFrameDeltaSeconds);
			m_elapsedRealTimeSeconds += schedule.acceptedFrameDeltaSeconds;

			if (schedule.droppedTimeSeconds > 0.0)
			{
				Log::Write(
					LogLevel::Warning,
					"FixedStepScheduler",
					"Dropped " + std::to_string(schedule.droppedTimeSeconds) +
					" seconds of simulation backlog.");
			}

			try
			{
				for (std::uint32_t updateIndex = 0;
					updateIndex < schedule.updateCount;
					++updateIndex)
				{
					const std::uint64_t tickIndex =
						schedule.firstTickIndex + updateIndex;
					FixedFrameContext fixedFrame;
					fixedFrame.tickIndex = tickIndex;
					fixedFrame.deltaMilliseconds = static_cast<float>(
						schedule.fixedDeltaSeconds * 1000.0);
					fixedFrame.fixedDeltaSeconds = static_cast<float>(
						schedule.fixedDeltaSeconds);
					fixedFrame.simulationTimeSeconds =
						static_cast<double>(tickIndex + 1) * schedule.fixedDeltaSeconds;
					fixedFrame.fps = m_fps.GetFps();
					fixedFrame.cpuPercentage = m_cpu.GetCpuPercentage();

					const Result<UpdateResult> updateResult =
						m_game->FixedUpdate(fixedFrame);
					if (!updateResult)
					{
						runtimeError = updateResult.GetError();
						break;
					}
					if (updateResult.Value() == UpdateResult::ExitRequested)
					{
						exitRequested = true;
						break;
					}
				}

				if (runtimeError.has_value() || exitRequested)
				{
					break;
				}

				RenderFrameContext renderFrame;
				renderFrame.frameIndex = m_frameIndex;
				renderFrame.completedFixedTickCount = schedule.completedTickCount;
				renderFrame.frameDeltaMilliseconds = static_cast<float>(
					schedule.acceptedFrameDeltaSeconds * 1000.0);
				renderFrame.frameDeltaSeconds = static_cast<float>(
					schedule.acceptedFrameDeltaSeconds);
				renderFrame.elapsedRealTimeSeconds = m_elapsedRealTimeSeconds;
				renderFrame.interpolationAlpha = static_cast<float>(
					schedule.interpolationAlpha);
				renderFrame.fps = m_fps.GetFps();
				renderFrame.cpuPercentage = m_cpu.GetCpuPercentage();

				const Result<void> rendered = m_game->Render(renderFrame);
				if (!rendered)
				{
					runtimeError = rendered.GetError();
					break;
				}
			}
			catch (const std::exception& exception)
			{
				runtimeError = ENGINE_ERROR(
					ErrorCode::UnexpectedFailure,
					"Application",
					std::string("Runtime loop threw an exception: ") + exception.what());
				break;
			}
			catch (...)
			{
				runtimeError = ENGINE_ERROR(
					ErrorCode::UnexpectedFailure,
					"Application",
					"Runtime loop threw an unknown exception.");
				break;
			}

			++m_frameIndex;
		}

		m_lifecycle = Lifecycle::Initialized;
		Shutdown();

		if (runtimeError.has_value())
		{
			Log::Write(LogLevel::Error, *runtimeError);
			return Result<int>::Failure(*runtimeError);
		}

		Log::Write(
			LogLevel::Info,
			"Application",
			"Runtime stopped after " + std::to_string(m_frameIndex) +
			" render frames and " +
			std::to_string(m_fixedStepScheduler.CompletedTickCount()) +
			" fixed ticks.");
		return Result<int>::Success(exitCode);
	}

	void Application::Shutdown() noexcept
	{
		if (m_lifecycle == Lifecycle::Stopped)
		{
			return;
		}

		// Mark each resource before releasing it so re-entrant shutdown remains
		// idempotent.
		if (m_gameNeedsShutdown && m_game)
		{
			m_gameNeedsShutdown = false;
			m_game->Shutdown();
		}

		if (m_cpuNeedsShutdown)
		{
			m_cpuNeedsShutdown = false;
			m_cpu.Shutdown();
		}

		m_window.Destroy();
		m_lifecycle = Lifecycle::Stopped;
	}

	void Application::RequestExit(int exitCode) noexcept
	{
		m_window.RequestClose(exitCode);
	}

	bool Application::IsInitialized() const noexcept
	{
		return m_lifecycle == Lifecycle::Initialized ||
			m_lifecycle == Lifecycle::Running;
	}

	const Win32Window& Application::Window() const noexcept
	{
		return m_window;
	}

}
