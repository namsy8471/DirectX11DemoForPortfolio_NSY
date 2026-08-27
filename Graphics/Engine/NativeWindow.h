#pragma once

// Forward declarations keep the game interface independent from Windows.h and
// its process-wide macro surface while preserving type-safe native handles.
struct HWND__;
struct HINSTANCE__;

namespace Engine
{
	struct NativeWindow final
	{
		HWND__* handle = nullptr;
		HINSTANCE__* instance = nullptr;
		int width = 0;
		int height = 0;

		explicit operator bool() const noexcept
		{
			return handle != nullptr && instance != nullptr;
		}
	};
}
