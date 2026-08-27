#include "Win32Window.h"

#include <cstdlib>
#include <exception>
#include <utility>

namespace Engine
{
	Win32Window::~Win32Window()
	{
		Destroy();
	}

	bool Win32Window::Create(const WindowConfig& config, MessageCallback callback)
	{
		if (IsOpen() || config.width <= 0 || config.height <= 0 ||
			config.className.empty())
		{
			return false;
		}

		m_messageCallback = std::move(callback);
		m_registeredClassName = config.className;
		m_nativeWindow.instance = GetModuleHandleW(nullptr);
		m_creatorThreadId = GetCurrentThreadId();
		m_nativeWindow.width = config.width;
		m_nativeWindow.height = config.height;
		m_callbackFailed = false;
		m_minimized = false;
		m_requestedExitCode.store(EXIT_SUCCESS, std::memory_order_relaxed);

		if (!m_nativeWindow.instance)
		{
			Destroy();
			return false;
		}

		WNDCLASSEXW windowClass{};
		windowClass.cbSize = sizeof(windowClass);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = &Win32Window::StaticWindowProc;
		windowClass.hInstance = m_nativeWindow.instance;
		windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		windowClass.hIconSm = windowClass.hIcon;
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.hbrBackground = reinterpret_cast<HBRUSH>(
			GetStockObject(BLACK_BRUSH));
		windowClass.lpszClassName = m_registeredClassName.c_str();

		if (RegisterClassExW(&windowClass) != 0)
		{
			m_registeredClass = true;
		}
		else if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
		{
			WNDCLASSEXW existingClass{};
			existingClass.cbSize = sizeof(existingClass);
			if (!GetClassInfoExW(
					m_nativeWindow.instance,
					m_registeredClassName.c_str(),
					&existingClass) ||
				existingClass.lpfnWndProc != &Win32Window::StaticWindowProc)
			{
				Destroy();
				return false;
			}
		}
		else
		{
			Destroy();
			return false;
		}

		DWORD style = WS_OVERLAPPEDWINDOW;
		DWORD extendedStyle = WS_EX_APPWINDOW;
		int x = CW_USEDEFAULT;
		int y = CW_USEDEFAULT;
		int windowWidth = config.width;
		int windowHeight = config.height;

		if (config.fullscreen)
		{
			// Borderless fullscreen avoids mutating the process-wide display mode.
			style = WS_POPUP;
			x = 0;
			y = 0;
			windowWidth = GetSystemMetrics(SM_CXSCREEN);
			windowHeight = GetSystemMetrics(SM_CYSCREEN);
			m_nativeWindow.width = windowWidth;
			m_nativeWindow.height = windowHeight;
		}
		else
		{
			if (!config.resizable)
			{
				style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
			}

			RECT windowRect{ 0, 0, config.width, config.height };
			if (!AdjustWindowRectEx(&windowRect, style, FALSE, extendedStyle))
			{
				Destroy();
				return false;
			}

			windowWidth = windowRect.right - windowRect.left;
			windowHeight = windowRect.bottom - windowRect.top;

			if (config.centered)
			{
				RECT workArea{};
				if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
				{
					x = workArea.left +
						((workArea.right - workArea.left) - windowWidth) / 2;
					y = workArea.top +
						((workArea.bottom - workArea.top) - windowHeight) / 2;
				}
			}
		}

		const HWND hwnd = CreateWindowExW(
			extendedStyle,
			m_registeredClassName.c_str(),
			config.title.c_str(),
			style,
			x,
			y,
			windowWidth,
			windowHeight,
			nullptr,
			nullptr,
			m_nativeWindow.instance,
			this);

		if (!hwnd)
		{
			Destroy();
			return false;
		}

		// WM_NCCREATE stores hwnd before CreateWindowExW returns. Assigning it
		// again makes the postcondition explicit and robust to message changes.
		m_nativeWindow.handle = hwnd;
		m_closeHandle.store(hwnd, std::memory_order_release);
		UpdateClientSize(hwnd);
		m_postQuitOnDestroy = true;

		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);

		if (!config.showCursor)
		{
			do
			{
				++m_cursorHideAdjustments;
			} while (ShowCursor(FALSE) >= 0);
		}

		return true;
	}

	void Win32Window::Destroy() noexcept
	{
		// HWND destruction and ShowCursor accounting are thread-affine. RequestClose
		// is the only operation intentionally callable from another thread.
		if (m_creatorThreadId != 0 && m_creatorThreadId != GetCurrentThreadId())
		{
			std::terminate();
		}

		// Programmatic teardown must not leave a stale WM_QUIT in a future loop.
		m_postQuitOnDestroy = false;

		const HWND hwnd = m_nativeWindow.handle;
		if (hwnd && IsWindow(hwnd))
		{
			if (!DestroyWindow(hwnd))
			{
				// Detach the callback before yielding ownership of a live HWND. This
				// prevents a late native message from dereferencing a dead C++ object.
				SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
				PostMessageW(hwnd, WM_CLOSE, 0, 0);
			}
		}

		m_nativeWindow.handle = nullptr;
		m_closeHandle.store(nullptr, std::memory_order_release);
		RestoreCursor();

		if (m_registeredClass && m_nativeWindow.instance &&
			!m_registeredClassName.empty())
		{
			UnregisterClassW(
				m_registeredClassName.c_str(),
				m_nativeWindow.instance);
		}

		m_messageCallback = {};
		m_registeredClassName.clear();
		m_registeredClass = false;
		m_minimized = false;
		m_cursorHideAdjustments = 0;
		m_creatorThreadId = 0;
		m_nativeWindow = {};
	}

	bool Win32Window::PumpMessages(int& exitCode) noexcept
	{
		MSG message{};
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				exitCode = static_cast<int>(message.wParam);
				return false;
			}

			TranslateMessage(&message);
			DispatchMessageW(&message);
		}

		if (m_callbackFailed)
		{
			exitCode = EXIT_FAILURE;
			return false;
		}

		if (!IsOpen())
		{
			exitCode = m_requestedExitCode.load(std::memory_order_relaxed);
			return false;
		}

		return true;
	}

	void Win32Window::WaitForActivity() const noexcept
	{
		WaitMessage();
	}

	void Win32Window::RequestClose(int exitCode) noexcept
	{
		const HWND hwnd = m_closeHandle.load(std::memory_order_acquire);
		if (hwnd != nullptr)
		{
			PostMessageW(
				hwnd,
				RequestCloseMessage,
				static_cast<WPARAM>(static_cast<INT_PTR>(exitCode)),
				reinterpret_cast<LPARAM>(this));
		}
	}

	bool Win32Window::IsOpen() const noexcept
	{
		return m_nativeWindow.handle != nullptr;
	}

	bool Win32Window::IsMinimized() const noexcept
	{
		return m_minimized;
	}

	NativeWindow Win32Window::GetNativeWindow() const noexcept
	{
		return m_nativeWindow;
	}

	LRESULT CALLBACK Win32Window::StaticWindowProc(
		HWND hwnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam) noexcept
	{
		Win32Window* window = reinterpret_cast<Win32Window*>(
			GetWindowLongPtrW(hwnd, GWLP_USERDATA));

		if (message == WM_NCCREATE)
		{
			const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
			window = create
				? static_cast<Win32Window*>(create->lpCreateParams)
				: nullptr;

			if (!window)
			{
				return FALSE;
			}

			SetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA,
				reinterpret_cast<LONG_PTR>(window));
			window->m_nativeWindow.handle = hwnd;
			window->m_closeHandle.store(hwnd, std::memory_order_release);
		}

		if (window)
		{
			return window->HandleMessage(hwnd, message, wParam, lParam);
		}

		return DefWindowProcW(hwnd, message, wParam, lParam);
	}

	LRESULT Win32Window::HandleMessage(
		HWND hwnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam) noexcept
	{
		if (message == WM_SIZE)
		{
			m_minimized = wParam == SIZE_MINIMIZED;
			if (!m_minimized)
			{
				UpdateClientSize(hwnd);
			}
		}

		if (message == RequestCloseMessage &&
			reinterpret_cast<Win32Window*>(lParam) == this)
		{
			m_requestedExitCode.store(
				static_cast<int>(static_cast<INT_PTR>(wParam)),
				std::memory_order_relaxed);
			DestroyWindow(hwnd);
			return 0;
		}

		LRESULT callbackResult = 0;
		bool handled = false;
		if (m_messageCallback)
		{
			try
			{
				handled = m_messageCallback(
					hwnd,
					message,
					wParam,
					lParam,
					callbackResult);
			}
			catch (...)
			{
				// C++ exceptions must never cross the Win32 callback ABI.
				m_callbackFailed = true;
				m_requestedExitCode.store(EXIT_FAILURE, std::memory_order_relaxed);
				if (message != WM_DESTROY && message != WM_NCDESTROY)
				{
					DestroyWindow(hwnd);
				}
				return 0;
			}
		}

		switch (message)
		{
		case WM_CLOSE:
			if (handled)
			{
				m_requestedExitCode.store(EXIT_SUCCESS, std::memory_order_relaxed);
				return callbackResult;
			}
			DestroyWindow(hwnd);
			return 0;

		case WM_DESTROY:
			if (m_postQuitOnDestroy)
			{
				PostQuitMessage(m_requestedExitCode.load(std::memory_order_relaxed));
			}
			return handled ? callbackResult : 0;

		case WM_NCDESTROY:
		{
			const LRESULT result = handled
				? callbackResult
				: DefWindowProcW(hwnd, message, wParam, lParam);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
			if (m_nativeWindow.handle == hwnd)
			{
				m_nativeWindow.handle = nullptr;
				m_closeHandle.store(nullptr, std::memory_order_release);
			}
			return result;
		}

		default:
			break;
		}

		return handled
			? callbackResult
			: DefWindowProcW(hwnd, message, wParam, lParam);
	}

	void Win32Window::UpdateClientSize(HWND hwnd) noexcept
	{
		RECT clientRect{};
		if (GetClientRect(hwnd, &clientRect))
		{
			m_nativeWindow.width = clientRect.right - clientRect.left;
			m_nativeWindow.height = clientRect.bottom - clientRect.top;
		}
	}

	void Win32Window::RestoreCursor() noexcept
	{
		while (m_cursorHideAdjustments > 0)
		{
			ShowCursor(TRUE);
			--m_cursorHideAdjustments;
		}
	}
}
