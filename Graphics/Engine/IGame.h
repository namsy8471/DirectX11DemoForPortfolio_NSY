#pragma once

#include "Engine/Core/Result.h"
#include "Engine/FrameContext.h"
#include "Engine/Platform/NativeWindow.h"

namespace Engine
{
	enum class UpdateResult
	{
		Continue,
		ExitRequested
	};

	// Application-facing game boundary. Rendering technology, scene ownership,
	// and game rules stay behind this interface.
	class IGame
	{
	public:
		virtual ~IGame() = default;

		virtual Result<void> Initialize(const NativeWindow& window) = 0;

		virtual Result<UpdateResult> FixedUpdate(
			const FixedFrameContext& frame) = 0;

		virtual Result<void> Render(const RenderFrameContext& frame) = 0;

		// Initialize may fail after acquiring only some resources, so Application
		// calls Shutdown after every attempted initialization.
		virtual void Shutdown() noexcept = 0;

	};
}
