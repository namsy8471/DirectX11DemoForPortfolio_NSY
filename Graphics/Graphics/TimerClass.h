#pragma once

class TimerClass
{
public:
	TimerClass();
	TimerClass(const TimerClass&);
	~TimerClass();

	bool Initialize();
	void Frame();

	float GetTime();

private:
	LARGE_INTEGER m_frequency;
	float m_ticksPerMs = 0;
	LARGE_INTEGER m_startTime;
	float m_frameTime = 0;
};