#include "stdafx.h"
#include "Engine/Diagnostics/CpuClass.h"

#include <algorithm>

CpuClass::~CpuClass()
{
	Shutdown();
}

bool CpuClass::Initialize() noexcept
{
	Shutdown();

	HQUERY queryHandle = nullptr;
	if (PdhOpenQuery(nullptr, 0, &queryHandle) != ERROR_SUCCESS)
	{
		return false;
	}

	HCOUNTER counterHandle = nullptr;
	if (PdhAddCounter(
		queryHandle,
		TEXT("\\Processor(_Total)\\% Processor Time"),
		0,
		&counterHandle) != ERROR_SUCCESS)
	{
		PdhCloseQuery(queryHandle);
		return false;
	}

	if (PdhCollectQueryData(queryHandle) != ERROR_SUCCESS)
	{
		PdhCloseQuery(queryHandle);
		return false;
	}

	m_queryHandle = queryHandle;
	m_counterHandle = counterHandle;
	m_canReadCpu = true;
	m_lastSampleTime = GetTickCount64();
	m_cpuUsage = 0;
	return true;
}

void CpuClass::Shutdown() noexcept
{
	if (m_queryHandle != nullptr)
	{
		PdhCloseQuery(m_queryHandle);
	}

	m_queryHandle = nullptr;
	m_counterHandle = nullptr;
	m_canReadCpu = false;
	m_lastSampleTime = 0;
	m_cpuUsage = 0;
}

void CpuClass::Frame() noexcept
{
	if (!m_canReadCpu || m_queryHandle == nullptr || m_counterHandle == nullptr)
	{
		return;
	}

	const ULONGLONG currentTime = GetTickCount64();
	if (currentTime - m_lastSampleTime < 1000)
	{
		return;
	}
	m_lastSampleTime = currentTime;

	if (PdhCollectQueryData(m_queryHandle) != ERROR_SUCCESS)
	{
		return;
	}

	PDH_FMT_COUNTERVALUE value = {};
	if (PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, nullptr, &value) != ERROR_SUCCESS ||
		(value.CStatus != PDH_CSTATUS_VALID_DATA && value.CStatus != PDH_CSTATUS_NEW_DATA))
	{
		return;
	}

	m_cpuUsage = std::clamp(value.longValue, 0L, 100L);
}

int CpuClass::GetCpuPercentage() const noexcept
{
	return m_canReadCpu ? static_cast<int>(m_cpuUsage) : 0;
}

bool CpuClass::IsAvailable() const noexcept
{
	return m_canReadCpu;
}
