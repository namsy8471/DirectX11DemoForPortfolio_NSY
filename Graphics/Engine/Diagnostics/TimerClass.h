#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

class TimerClass final
{
public:
	TimerClass() noexcept = default;
	~TimerClass() = default;

	TimerClass(const TimerClass&) = delete;
	TimerClass& operator=(const TimerClass&) = delete;

	[[nodiscard]] bool Initialize() noexcept;
	void Reset() noexcept;
	void Frame() noexcept;

	[[nodiscard]] float GetTime() const noexcept;

private:
	LARGE_INTEGER m_startTime{};
	double m_millisecondsPerTick = 0.0;
	float m_frameTimeMilliseconds = 0.0f;
};
