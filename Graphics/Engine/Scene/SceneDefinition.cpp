#include "SceneDefinition.h"

#include <type_traits>
#include <utility>

namespace
{
	Engine::Scene::GameObjectDefinition MakeGameObject(
		std::string name,
		std::wstring modelPath,
		std::wstring texturePath,
		const DirectX::XMFLOAT3& position,
		const DirectX::XMFLOAT3& rotationDegrees,
		const DirectX::XMFLOAT3& scale)
	{
		Engine::Scene::GameObjectDefinition definition{
			std::move(name),
			std::move(modelPath),
			std::move(texturePath),
			{}};

		definition.transform.SetPosition(position);
		definition.transform.SetRotationDegrees(
			rotationDegrees.x,
			rotationDegrees.y,
			rotationDegrees.z);
		definition.transform.SetScale(scale);
		return definition;
	}
}

namespace Engine::Scene
{
	SceneDefinition SceneDefinition::CreatePortfolioDemo()
	{
		SceneDefinition scene;
		scene.gameObjects.reserve(4);

		// Preserve the original portfolio scene and its authored model transforms.
		scene.gameObjects.push_back(MakeGameObject(
			"bear",
			L"./data/bear.obj",
			L"./data/bear.dds",
			{3.0f, -2.5f, -1.0f},
			{0.0f, 0.0f, 0.0f},
			{1.0f, 1.0f, 1.0f}));

		scene.gameObjects.push_back(MakeGameObject(
			"wild_turkey_male",
			L"./data/11560_wild_turkey_male_v2_l2.obj",
			L"./data/11560_wild_turkey_male_v2_l2.dds",
			{8.0f, -2.5f, -1.0f},
			{90.0f, 90.0f, 0.0f},
			{0.01f, 0.01f, 0.01f}));

		scene.gameObjects.push_back(MakeGameObject(
			"wild_turkey_female",
			L"./data/11561_Turkey - Wild_female_v4_l1.obj",
			L"./data/11561_Turkey - Wild_female_v4_l1.dds",
			{0.0f, -2.5f, -1.0f},
			{90.0f, 90.0f, 0.0f},
			{0.01f, 0.01f, 0.01f}));

		scene.gameObjects.push_back(MakeGameObject(
			"horse",
			L"./data/Horse.obj",
			L"./data/Horse.dds",
			{5.0f, -2.5f, -1.0f},
			{90.0f, 0.0f, 0.0f},
			{0.002f, 0.002f, 0.002f}));

		scene.terrain = {
			"./data/heightmap01.bmp",
			L"./data/dirt01.dds",
			{-100.0f, -3.0f, -100.0f}};

		scene.fire = {
			"./data/square.txt",
			L"./data/fire01.dds",
			L"./data/noise01.dds",
			L"./data/alpha01.dds",
			{5.5f, 5.5f, 1.0f}};

		scene.ui = {
			L"./data/Crosshair.dds",
			L"./data/mouse.dds"};

		scene.foliage = {
			L"./data/grass.dds",
			2000};

		scene.audio = {
			"./data/birdSoundBGM.wav",
			"./data/sound01.wav",
			"./data/GunShotSound.wav"};

		scene.shadowMap = {1024, 1024};
		return scene;
	}

	static_assert(std::is_copy_constructible_v<SceneDefinition>);
	static_assert(std::is_copy_assignable_v<SceneDefinition>);
	static_assert(std::is_move_constructible_v<SceneDefinition>);
	static_assert(std::is_move_assignable_v<SceneDefinition>);
}
