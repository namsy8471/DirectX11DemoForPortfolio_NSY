#pragma once

#include "Engine/Scene/Transform.h"

#include <string>
#include <vector>

namespace Engine::Scene
{
	struct GameObjectDefinition
	{
		std::string name;
		std::wstring modelPath;
		std::wstring texturePath;
		Transform transform;
	};

	struct TerrainDefinition
	{
		std::string heightMapPath;
		std::wstring texturePath;
		DirectX::XMFLOAT3 worldOffset{0.0f, 0.0f, 0.0f};
	};

	struct FireDefinition
	{
		std::string modelPath;
		std::wstring fireTexturePath;
		std::wstring noiseTexturePath;
		std::wstring alphaTexturePath;
		DirectX::XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
	};

	struct UiDefinition
	{
		std::wstring crosshairTexturePath;
		std::wstring cursorTexturePath;
	};

	struct FoliageDefinition
	{
		std::wstring texturePath;
		int count = 0;
	};

	struct AudioDefinition
	{
		std::string bgmPath;
		std::string startupSfxPath;
		std::string actionSfxPath;
	};

	struct ShadowMapDefinition
	{
		int width = 0;
		int height = 0;
	};

	struct SceneDefinition
	{
		std::vector<GameObjectDefinition> gameObjects;
		TerrainDefinition terrain;
		FireDefinition fire;
		UiDefinition ui;
		FoliageDefinition foliage;
		AudioDefinition audio;
		ShadowMapDefinition shadowMap;

		[[nodiscard]] static SceneDefinition CreatePortfolioDemo();
	};
}
