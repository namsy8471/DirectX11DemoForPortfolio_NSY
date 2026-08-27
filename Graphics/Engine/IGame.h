#pragma once

#include "FrameContext.h"
#include "NativeWindow.h"

namespace Engine
{
	enum class UpdateResult
	{
		Continue,
		ExitRequested,
		Failure
	};

	// Application-facing game boundary. Rendering technology, scene ownership,
	// and game rules stay behind this interface.
	class IGame
	{
	public:
		virtual ~IGame() = default;

		virtual bool Initialize(const NativeWindow& window) = 0;

		virtual UpdateResult Update(const FrameContext& frame) = 0;

		// Returning false reports a rendering failure.
		virtual bool Render(const FrameContext& frame) = 0;

		// Initialize may fail after acquiring only some resources, so Application
		// calls Shutdown after every attempted initialization.
		virtual void Shutdown() noexcept = 0;

	};
}
