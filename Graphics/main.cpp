#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "Engine/Application.h"
#include "graphicsclass.h"

#include <memory>
#include <utility>

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	Engine::WindowConfig window;
	window.title = L"DirectX 11 Portfolio Engine";
	window.className = L"DirectX11PortfolioEngineWindow";
	window.width = 800;
	window.height = 600;
	window.fullscreen = false;
	window.resizable = false;
	window.showCursor = false;

	auto sceneDefinition = Engine::Scene::SceneDefinition::CreatePortfolioDemo();
	Engine::Application application(
		std::make_unique<PortfolioGame>(std::move(sceneDefinition)),
		window);

	return application.Run();
}
