#pragma once

#include "FrameContext.h"
#include "IGame.h"
#include "Win32Window.h"

#include "../CpuClass.h"
#include "../FpsClass.h"
#include "../TimerClass.h"

#include <cstdint>
#include <memory>

namespace Engine
{
	class Application final
	{
	public:
		explicit Application(
			std::unique_ptr<IGame> game,
			WindowConfig windowConfig = {});
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		bool Initialize();
		int Run();
		void Shutdown() noexcept;

		void RequestExit(int exitCode = 0) noexcept;
		bool IsInitialized() const noexcept;
		const Win32Window& Window() const noexcept;

	private:
		enum class Lifecycle
		{
			Constructed,
			Initializing,
			Initialized,
			Running,
			Stopped
		};

		WindowConfig m_windowConfig;
		Win32Window m_window;
		std::unique_ptr<IGame> m_game;
		::FpsClass m_fps;
		::CpuClass m_cpu;
		::TimerClass m_timer;

		Lifecycle m_lifecycle = Lifecycle::Constructed;
		bool m_gameNeedsShutdown = false;
		bool m_cpuNeedsShutdown = false;
		std::uint64_t m_frameIndex = 0;
		double m_elapsedTimeSeconds = 0.0;
	};
}
