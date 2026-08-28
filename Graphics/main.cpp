#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "Engine/Application.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/ScopeExit.h"
#include "Game/graphicsclass.h"

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <utility>

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	Engine::LogConfig logConfig;
	logConfig.filePath = std::filesystem::path("logs") / "PortfolioEngine.log";
	logConfig.minimumLevel = Engine::LogLevel::Info;
	logConfig.writeToStandardError = true;

	const Engine::Result<void> logInitialized = Engine::Log::Initialize(logConfig);
	if (!logInitialized)
	{
		const std::string message = logInitialized.GetError().ToString();
		MessageBoxA(nullptr, message.c_str(), "Portfolio Engine", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}
	[[maybe_unused]] auto shutdownLog = Engine::MakeScopeExit([]() noexcept {
		Engine::Log::Shutdown();
	});

	Engine::WindowConfig window;
	window.title = L"DirectX 11 Portfolio Engine";
	window.className = L"DirectX11PortfolioEngineWindow";
	window.width = 800;
	window.height = 600;
	window.fullscreen = false;
	window.resizable = false;
	window.showCursor = false;

	auto sceneDefinition = Engine::Scene::SceneDefinition::CreatePortfolioDemo();
	Engine::Runtime::FixedStepConfig runtime;
	runtime.fixedDeltaSeconds = 1.0 / 60.0;
	runtime.maximumFrameDeltaSeconds = 0.1;
	runtime.maximumUpdatesPerFrame = 8;

	Engine::Application application(
		std::make_unique<PortfolioGame>(std::move(sceneDefinition)),
		window,
		runtime);

	const Engine::Result<int> runResult = application.Run();
	if (!runResult)
	{
		Engine::Log::Write(Engine::LogLevel::Fatal, runResult.GetError());
		const std::string message = runResult.GetError().ToString();
		MessageBoxA(nullptr, message.c_str(), "Portfolio Engine", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	return runResult.Value();
}
