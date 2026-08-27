#pragma once

#include "Engine/IGame.h"
#include "Engine/Core/ScopedResource.h"
#include "Engine/Scene/SceneDefinition.h"

#include <memory>
#include <vector>

#include <directxmath.h>

class BitmapClass;
class CameraClass;
class D3DClass;
class FireShaderClass;
class FoliageClass;
class FoliageShaderClass;
class InputClass;
class LightClass;
class ModelClass;
class ModelClassForNoiseFilter;
class SkyDomeClass;
class SkyDomeShaderClass;
class SoundClass;
class TerrainClass;
class TextClass;
class TextureShaderClass;

namespace Engine
{
	class FirstPersonCameraController;
	namespace Rendering
	{
		class SoftShadowPipeline;
	}
	namespace Scene
	{
		class GameObject;
	}
}

// Concrete game supplied to the reusable Engine::Application runtime.
class GraphicsClass final : public Engine::IGame
{
public:
	explicit GraphicsClass(
		Engine::Scene::SceneDefinition sceneDefinition =
			Engine::Scene::SceneDefinition::CreatePortfolioDemo());
	~GraphicsClass() override;

	GraphicsClass(const GraphicsClass&) = delete;
	GraphicsClass& operator=(const GraphicsClass&) = delete;

	bool Initialize(const Engine::NativeWindow& window) override;
	Engine::UpdateResult Update(const Engine::FrameContext& frame) override;
	bool Render(const Engine::FrameContext& frame) override;
	void Shutdown() noexcept override;

private:
	struct RenderSettings
	{
		static constexpr bool Vsync = false;
		static constexpr bool FullScreen = false;
		static constexpr float ScreenDepth = 1000.0f;
		static constexpr float ScreenNear = 0.1f;
	};

	enum class Stage
	{
		Title,
		Hunt
	};

	bool InitializeResources(
		int screenWidth,
		int screenHeight,
		HWND__* window,
		HINSTANCE__* instance);
	Engine::UpdateResult UpdateScene(const Engine::FrameContext& frame);
	Engine::UpdateResult HandleInput(float frameTime);
	void TestIntersection();

	bool RenderScene(float frameTime);

	Stage m_stage = Stage::Title;
	unsigned int m_objectCount = 0;
	unsigned int m_polygonCount = 0;
	bool m_primaryActionWasDown = false;
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	float m_lightPositionX = -5.0f;
	float m_fireAnimationTime = 0.0f;
	Engine::Scene::SceneDefinition m_sceneDefinition;

	Engine::ScopedResource<D3DClass> m_d3d;
	std::unique_ptr<CameraClass> m_camera;
	Engine::ScopedResource<InputClass> m_input;
	std::unique_ptr<Engine::FirstPersonCameraController> m_cameraController;
	Engine::ScopedResource<SoundClass> m_sound;

	std::vector<std::unique_ptr<Engine::Scene::GameObject>> m_gameObjects;
	Engine::ScopedResource<BitmapClass> m_mouseCursor;
	std::vector<Engine::ScopedResource<BitmapClass>> m_crosshairs;
	Engine::ScopedResource<TextureShaderClass> m_textureShader;
	Engine::ScopedResource<TextClass> m_text;

	Engine::ScopedResource<ModelClassForNoiseFilter> m_fireModel;
	Engine::ScopedResource<FireShaderClass> m_fireShader;

	Engine::ScopedResource<TerrainClass> m_terrain;
	std::unique_ptr<LightClass> m_light;
	std::unique_ptr<Engine::Rendering::SoftShadowPipeline> m_softShadowPipeline;

	Engine::ScopedResource<SkyDomeClass> m_skyDome;
	Engine::ScopedResource<SkyDomeShaderClass> m_skyDomeShader;
	Engine::ScopedResource<FoliageClass> m_foliage;
	Engine::ScopedResource<FoliageShaderClass> m_foliageShader;

};

using PortfolioGame = GraphicsClass;
