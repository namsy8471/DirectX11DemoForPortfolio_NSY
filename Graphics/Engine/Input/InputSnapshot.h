#pragma once

namespace Engine::Input
{
	// Per-frame, device-independent intent produced by the platform input layer.
	struct InputSnapshot final
	{
		bool escapePressed = false;
		bool primaryActionDown = false;
		int cursorX = 0;
		int cursorY = 0;
		float moveRight = 0.0f;
		float moveForward = 0.0f;
		float lookX = 0.0f;
		float lookY = 0.0f;
	};
}
