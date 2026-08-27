#pragma once

#pragma comment(lib, "pdh.lib")

#include <pdh.h>
#include <pdhmsg.h>

class CpuClass
{
public:
	CpuClass() = default;
	CpuClass(const CpuClass&) = delete;
	CpuClass& operator=(const CpuClass&) = delete;
	CpuClass(CpuClass&&) = delete;
	CpuClass& operator=(CpuClass&&) = delete;
	~CpuClass();

	[[nodiscard]] bool Initialize() noexcept;
	void Shutdown() noexcept;
	void Frame() noexcept;
	[[nodiscard]] int GetCpuPercentage() const noexcept;
	[[nodiscard]] bool IsAvailable() const noexcept;

private:
	bool m_canReadCpu = false;
	HQUERY m_queryHandle = nullptr;
	HCOUNTER m_counterHandle = nullptr;
	ULONGLONG m_lastSampleTime = 0;
	long m_cpuUsage = 0;
};
