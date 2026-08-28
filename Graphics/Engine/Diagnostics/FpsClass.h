#pragma once

#include <chrono>

class FpsClass final
{
public:
	FpsClass() noexcept = default;
	~FpsClass() = default;

	FpsClass(const FpsClass&) = delete;
	FpsClass& operator=(const FpsClass&) = delete;

	void Initialize() noexcept;
	void Frame() noexcept;
	[[nodiscard]] int GetFps() const noexcept;

private:
	using Clock = std::chrono::steady_clock;
	Clock::time_point m_sampleStart{};
	int m_fps = 0;
	int m_frameCount = 0;
};
