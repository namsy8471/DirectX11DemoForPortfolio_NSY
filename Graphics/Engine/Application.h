#pragma once

#include "Engine/Core/Result.h"
#include "Engine/FrameContext.h"
#include "Engine/IGame.h"
#include "Engine/Platform/Win32Window.h"
#include "Engine/Runtime/FixedStepScheduler.h"

#include "Engine/Diagnostics/CpuClass.h"
#include "Engine/Diagnostics/FpsClass.h"
#include "Engine/Diagnostics/TimerClass.h"

#include <cstdint>
#include <memory>

namespace Engine
{
	class Application final
	{
	public:
		explicit Application(
			std::unique_ptr<IGame> game,
			WindowConfig windowConfig = {},
			Runtime::FixedStepConfig fixedStepConfig = {});
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		Result<void> Initialize();
		Result<int> Run();
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
		Runtime::FixedStepConfig m_fixedStepConfig;
		Runtime::FixedStepScheduler m_fixedStepScheduler;
		Win32Window m_window;
		std::unique_ptr<IGame> m_game;
		::FpsClass m_fps;
		::CpuClass m_cpu;
		::TimerClass m_timer;

		Lifecycle m_lifecycle = Lifecycle::Constructed;
		bool m_gameNeedsShutdown = false;
		bool m_cpuNeedsShutdown = false;
		std::uint64_t m_frameIndex = 0;
		double m_elapsedRealTimeSeconds = 0.0;
	};
}
