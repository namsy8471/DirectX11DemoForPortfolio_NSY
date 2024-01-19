#include "stdafx.h"
#include "TimerClass.h"


TimerClass::TimerClass()
{
}


TimerClass::TimerClass(const TimerClass& other)
{
}


TimerClass::~TimerClass()
{
}


bool TimerClass::Initialize()
{
	// �� �ý����� ���� Ÿ�̸Ӹ� �����ϴ��� Ȯ���Ͻʽÿ�.
	/*QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if (m_frequency == 0)
	{
		return false;
	}*/

	//// �� ī���Ͱ� �� �и� �ʸ��� ƽ�ϴ� Ƚ���� Ȯ���մϴ�.
	//m_ticksPerMs = (float)(m_frequency / 1000);

	//QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	QueryPerformanceFrequency(&m_frequency);
	if (m_frequency.QuadPart == 0)
	{
		return false;
	}

	// �� ī���Ͱ� �� �и� �ʸ��� ƽ�ϴ� Ƚ���� Ȯ���մϴ�.
	m_ticksPerMs = (float)(m_frequency.QuadPart / 1000);

	QueryPerformanceCounter(&m_startTime);

	return true;
}


void TimerClass::Frame()
{
	LARGE_INTEGER currentTime;

	QueryPerformanceCounter(&currentTime);

	float timeDifference = (float)(currentTime.QuadPart - m_startTime.QuadPart);

	m_frameTime = timeDifference / m_ticksPerMs;

	QueryPerformanceCounter(&m_startTime);
}


float TimerClass::GetTime()
{
	return m_frameTime;
}