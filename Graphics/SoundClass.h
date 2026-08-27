#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <wrl/client.h>

class SoundClass
{
private:
	struct WaveHeaderType
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};

public:
	SoundClass() = default;
	SoundClass(const SoundClass&) = delete;
	SoundClass& operator=(const SoundClass&) = delete;
	SoundClass(SoundClass&&) = delete;
	SoundClass& operator=(SoundClass&&) = delete;
	~SoundClass();

	bool Initialize(HWND hwnd);
	bool Initialize(
		HWND hwnd,
		const char* startupSoundPath,
		const char* backgroundMusicPath,
		const char* actionSoundPath);
	void Shutdown() noexcept;

	void PlaySoundForBGM();
	void PlaySoundForSFX();

private:
	bool InitializeDirectSound(HWND hwnd);
	bool LoadWaveFile(const char* filename, Microsoft::WRL::ComPtr<IDirectSoundBuffer8>& secondaryBuffer);
	bool PlayWaveFile();

private:
	Microsoft::WRL::ComPtr<IDirectSound8> m_directSound;
	Microsoft::WRL::ComPtr<IDirectSoundBuffer> m_primaryBuffer;
	Microsoft::WRL::ComPtr<IDirectSoundBuffer8> m_secondaryBuffer1;
	Microsoft::WRL::ComPtr<IDirectSoundBuffer8> m_secondaryBuffer2;
	Microsoft::WRL::ComPtr<IDirectSoundBuffer8> m_secondaryBuffer3;
};
