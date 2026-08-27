#include "FpsClass.h"

#include <algorithm>
#include <cmath>

void FpsClass::Initialize() noexcept
{
	m_fps = 0;
	m_frameCount = 0;
	m_sampleStart = Clock::now();
}

void FpsClass::Frame() noexcept
{
	++m_frameCount;
	const Clock::time_point now = Clock::now();
	const std::chrono::duration<double> elapsed = now - m_sampleStart;
	if (elapsed.count() < 1.0)
	{
		return;
	}

	m_fps = static_cast<int>(std::lround(
		static_cast<double>(m_frameCount) / elapsed.count()));
	m_fps = (std::max)(m_fps, 0);
	m_frameCount = 0;
	m_sampleStart = now;
}

int FpsClass::GetFps() const noexcept
{
	return m_fps;
}
