#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include "NativeWindow.h"

#include <atomic>
#include <functional>
#include <string>

namespace Engine
{
	struct WindowConfig final
	{
		std::wstring title = L"Engine";
		std::wstring className = L"DirectX11PortfolioEngineWindow";
		int width = 800;
		int height = 600;
		bool fullscreen = false;
		// Swap-chain resizing is deliberately outside this minimal engine sample.
		bool resizable = false;
		bool centered = true;
		bool showCursor = true;
	};

	class Win32Window final
	{
	public:
		// The native window is owner-thread-affine. RequestClose is the sole
		// cross-thread-safe operation while the caller guarantees this object's
		// lifetime; destruction on another thread terminates.
		using MessageCallback = std::function<bool(
			HWND,
			UINT,
			WPARAM,
			LPARAM,
			LRESULT&)>;

		Win32Window() noexcept = default;
		~Win32Window();

		Win32Window(const Win32Window&) = delete;
		Win32Window& operator=(const Win32Window&) = delete;
		Win32Window(Win32Window&&) = delete;
		Win32Window& operator=(Win32Window&&) = delete;

		bool Create(const WindowConfig& config, MessageCallback callback);
		void Destroy() noexcept;

		// Drains the thread message queue. False means that WM_QUIT was received
		// or the native window no longer exists.
		bool PumpMessages(int& exitCode) noexcept;
		void WaitForActivity() const noexcept;
		void RequestClose(int exitCode = 0) noexcept;

		bool IsOpen() const noexcept;
		bool IsMinimized() const noexcept;
		NativeWindow GetNativeWindow() const noexcept;

	private:
		static constexpr UINT RequestCloseMessage = WM_APP + 0x31u;

		static LRESULT CALLBACK StaticWindowProc(
			HWND hwnd,
			UINT message,
			WPARAM wParam,
			LPARAM lParam) noexcept;

		LRESULT HandleMessage(
			HWND hwnd,
			UINT message,
			WPARAM wParam,
			LPARAM lParam) noexcept;

		void UpdateClientSize(HWND hwnd) noexcept;
		void RestoreCursor() noexcept;

		NativeWindow m_nativeWindow;
		MessageCallback m_messageCallback;
		std::wstring m_registeredClassName;
		bool m_registeredClass = false;
		bool m_postQuitOnDestroy = false;
		bool m_minimized = false;
		bool m_callbackFailed = false;
		int m_cursorHideAdjustments = 0;
		std::atomic<int> m_requestedExitCode{0};
		std::atomic<HWND> m_closeHandle{nullptr};
		DWORD m_creatorThreadId = 0;
	};
}
