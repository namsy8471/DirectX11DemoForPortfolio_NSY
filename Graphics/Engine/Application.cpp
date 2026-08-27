#include "Application.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace Engine
{
	Application::Application(
		std::unique_ptr<IGame> game,
		WindowConfig windowConfig)
		: m_windowConfig(std::move(windowConfig)),
		  m_game(std::move(game))
	{
	}

	Application::~Application()
	{
		Shutdown();
	}

	bool Application::Initialize()
	{
		if (m_lifecycle == Lifecycle::Initialized ||
			m_lifecycle == Lifecycle::Running)
		{
			return true;
		}

		if (m_lifecycle != Lifecycle::Constructed || !m_game)
		{
			return false;
		}

		m_lifecycle = Lifecycle::Initializing;

		if (!m_window.Create(m_windowConfig, {}))
		{
			m_lifecycle = Lifecycle::Stopped;
			return false;
		}

		// Shutdown is required even when Initialize returns false because a game
		// may have acquired only a prefix of its resources.
		m_gameNeedsShutdown = true;
		bool gameInitialized = false;
		try
		{
			gameInitialized = m_game->Initialize(m_window.GetNativeWindow());
		}
		catch (...)
		{
			gameInitialized = false;
		}

		if (!gameInitialized)
		{
			Shutdown();
			return false;
		}

		// Start telemetry only after resource loading. Otherwise the first frame
		// would include the entire initialization duration in its delta time.
		if (!m_timer.Initialize())
		{
			Shutdown();
			return false;
		}
		m_fps.Initialize();
		// CPU telemetry is optional; the game loop remains usable when PDH is
		// unavailable on the host.
		m_cpuNeedsShutdown = m_cpu.Initialize();

		m_frameIndex = 0;
		m_elapsedTimeSeconds = 0.0;
		m_lifecycle = Lifecycle::Initialized;
		return true;
	}

	int Application::Run()
	{
		if (m_lifecycle == Lifecycle::Constructed && !Initialize())
		{
			return EXIT_FAILURE;
		}

		if (m_lifecycle != Lifecycle::Initialized)
		{
			return EXIT_FAILURE;
		}

		m_lifecycle = Lifecycle::Running;
		int exitCode = EXIT_SUCCESS;
		bool wasSuspended = false;
		m_timer.Reset();
		m_fps.Initialize();

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
				wasSuspended = false;
			}

			m_timer.Frame();
			m_fps.Frame();
			m_cpu.Frame();

			// Clamp long stalls (debug breaks, window dragging, device hiccups) so a
			// single frame cannot teleport gameplay state or destabilize simulation.
			const float deltaMilliseconds = std::clamp(
				m_timer.GetTime(),
				0.0f,
				100.0f);
			const float deltaSeconds = deltaMilliseconds * 0.001f;
			m_elapsedTimeSeconds += static_cast<double>(deltaSeconds);

			FrameContext frame;
			frame.frameIndex = m_frameIndex;
			frame.deltaMilliseconds = deltaMilliseconds;
			frame.deltaTimeSeconds = deltaSeconds;
			frame.elapsedTimeSeconds = m_elapsedTimeSeconds;
			frame.fps = m_fps.GetFps();
			frame.cpuPercentage = m_cpu.GetCpuPercentage();

			try
			{
				const UpdateResult updateResult = m_game->Update(frame);
				if (updateResult == UpdateResult::ExitRequested)
				{
					break;
				}
				if (updateResult == UpdateResult::Failure)
				{
					exitCode = EXIT_FAILURE;
					break;
				}

				if (!m_game->Render(frame))
				{
					exitCode = EXIT_FAILURE;
					break;
				}
			}
			catch (...)
			{
				exitCode = EXIT_FAILURE;
				break;
			}

			++m_frameIndex;
		}

		m_lifecycle = Lifecycle::Initialized;
		Shutdown();
		return exitCode;
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
