#include "Engine/Diagnostics/TimerClass.h"

bool TimerClass::Initialize() noexcept
{
	LARGE_INTEGER frequency{};
	if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0)
	{
		m_millisecondsPerTick = 0.0;
		m_frameTimeMilliseconds = 0.0f;
		return false;
	}

	m_millisecondsPerTick = 1000.0 / static_cast<double>(frequency.QuadPart);
	Reset();
	return true;
}

void TimerClass::Reset() noexcept
{
	QueryPerformanceCounter(&m_startTime);
	m_frameTimeMilliseconds = 0.0f;
}

void TimerClass::Frame() noexcept
{
	if (m_millisecondsPerTick <= 0.0)
	{
		m_frameTimeMilliseconds = 0.0f;
		return;
	}

	LARGE_INTEGER currentTime{};
	QueryPerformanceCounter(&currentTime);
	const auto elapsedTicks = currentTime.QuadPart - m_startTime.QuadPart;
	m_frameTimeMilliseconds = static_cast<float>(
		static_cast<double>(elapsedTicks) * m_millisecondsPerTick);
	m_startTime = currentTime;
}

float TimerClass::GetTime() const noexcept
{
	return m_frameTimeMilliseconds;
}
