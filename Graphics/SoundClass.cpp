#include "SoundClass.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace
{
	bool HasFourCc(const char (&value)[4], const char (&expected)[5]) noexcept
	{
		return std::memcmp(value, expected, 4) == 0;
	}
}

SoundClass::~SoundClass()
{
	Shutdown();
}

bool SoundClass::Initialize(HWND hwnd)
{
	return Initialize(
		hwnd,
		"./data/sound01.wav",
		"./data/birdSoundBGM.wav",
		"./data/GunShotSound.wav");
}

bool SoundClass::Initialize(
	HWND hwnd,
	const char* startupSoundPath,
	const char* backgroundMusicPath,
	const char* actionSoundPath)
{
	Shutdown();

	if (!InitializeDirectSound(hwnd) ||
		!LoadWaveFile(startupSoundPath, m_secondaryBuffer1) ||
		!LoadWaveFile(backgroundMusicPath, m_secondaryBuffer2) ||
		!LoadWaveFile(actionSoundPath, m_secondaryBuffer3) ||
		!PlayWaveFile())
	{
		Shutdown();
		return false;
	}

	return true;
}

void SoundClass::Shutdown() noexcept
{
	m_secondaryBuffer3.Reset();
	m_secondaryBuffer2.Reset();
	m_secondaryBuffer1.Reset();
	m_primaryBuffer.Reset();
	m_directSound.Reset();
}

bool SoundClass::InitializeDirectSound(HWND hwnd)
{
	if (FAILED(DirectSoundCreate8(nullptr, m_directSound.ReleaseAndGetAddressOf(), nullptr)))
	{
		return false;
	}

	if (FAILED(m_directSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
	{
		return false;
	}

	DSBUFFERDESC bufferDesc = {};
	bufferDesc.dwSize = sizeof(bufferDesc);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.guid3DAlgorithm = GUID_NULL;
	if (FAILED(m_directSound->CreateSoundBuffer(
		&bufferDesc,
		m_primaryBuffer.ReleaseAndGetAddressOf(),
		nullptr)))
	{
		return false;
	}

	WAVEFORMATEX waveFormat = {};
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = static_cast<WORD>((waveFormat.wBitsPerSample / 8) * waveFormat.nChannels);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

	return SUCCEEDED(m_primaryBuffer->SetFormat(&waveFormat));
}

bool SoundClass::LoadWaveFile(const char* filename, ComPtr<IDirectSoundBuffer8>& secondaryBuffer)
{
	secondaryBuffer.Reset();
	if (filename == nullptr || !m_directSound)
	{
		return false;
	}

	std::ifstream file(filename, std::ios::binary);
	if (!file)
	{
		return false;
	}

	WaveHeaderType header = {};
	file.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (!file ||
		!HasFourCc(header.chunkId, "RIFF") ||
		!HasFourCc(header.format, "WAVE") ||
		!HasFourCc(header.subChunkId, "fmt ") ||
		!HasFourCc(header.dataChunkId, "data") ||
		header.audioFormat != WAVE_FORMAT_PCM ||
		header.numChannels != 2 ||
		header.sampleRate != 44100 ||
		header.bitsPerSample != 16 ||
		header.dataSize == 0)
	{
		return false;
	}

	const std::streampos dataStart = file.tellg();
	file.seekg(0, std::ios::end);
	const std::streampos fileEnd = file.tellg();
	if (dataStart < 0 || fileEnd < dataStart ||
		static_cast<unsigned long long>(fileEnd - dataStart) < header.dataSize)
	{
		return false;
	}
	file.seekg(dataStart);

	std::vector<unsigned char> waveData(header.dataSize);
	file.read(reinterpret_cast<char*>(waveData.data()), static_cast<std::streamsize>(waveData.size()));
	if (!file)
	{
		return false;
	}

	WAVEFORMATEX waveFormat = {};
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = header.sampleRate;
	waveFormat.wBitsPerSample = header.bitsPerSample;
	waveFormat.nChannels = header.numChannels;
	waveFormat.nBlockAlign = header.blockAlign;
	waveFormat.nAvgBytesPerSec = header.bytesPerSecond;

	DSBUFFERDESC bufferDesc = {};
	bufferDesc.dwSize = sizeof(bufferDesc);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = header.dataSize;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	ComPtr<IDirectSoundBuffer> temporaryBuffer;
	if (FAILED(m_directSound->CreateSoundBuffer(&bufferDesc, temporaryBuffer.GetAddressOf(), nullptr)))
	{
		return false;
	}

	ComPtr<IDirectSoundBuffer8> loadedBuffer;
	if (FAILED(temporaryBuffer->QueryInterface(
		IID_IDirectSoundBuffer8,
		reinterpret_cast<void**>(loadedBuffer.GetAddressOf()))))
	{
		return false;
	}

	void* firstRegion = nullptr;
	DWORD firstRegionSize = 0;
	void* secondRegion = nullptr;
	DWORD secondRegionSize = 0;
	if (FAILED(loadedBuffer->Lock(
		0,
		header.dataSize,
		&firstRegion,
		&firstRegionSize,
		&secondRegion,
		&secondRegionSize,
		0)))
	{
		return false;
	}

	const size_t lockedSize = static_cast<size_t>(firstRegionSize) + secondRegionSize;
	if (lockedSize < waveData.size())
	{
		loadedBuffer->Unlock(firstRegion, firstRegionSize, secondRegion, secondRegionSize);
		return false;
	}

	const size_t firstCopySize = (std::min)(static_cast<size_t>(firstRegionSize), waveData.size());
	if (firstCopySize > 0)
	{
		std::memcpy(firstRegion, waveData.data(), firstCopySize);
	}
	if (secondRegionSize > 0 && firstCopySize < waveData.size())
	{
		std::memcpy(
			secondRegion,
			waveData.data() + firstCopySize,
			waveData.size() - firstCopySize);
	}

	if (FAILED(loadedBuffer->Unlock(firstRegion, firstRegionSize, secondRegion, secondRegionSize)))
	{
		return false;
	}

	secondaryBuffer = loadedBuffer;
	return true;
}

bool SoundClass::PlayWaveFile()
{
	if (!m_secondaryBuffer1)
	{
		return false;
	}

	return SUCCEEDED(m_secondaryBuffer1->SetCurrentPosition(0)) &&
		SUCCEEDED(m_secondaryBuffer1->SetVolume(-2500)) &&
		SUCCEEDED(m_secondaryBuffer1->Play(0, 0, 0));
}

void SoundClass::PlaySoundForBGM()
{
	if (!m_secondaryBuffer2)
	{
		return;
	}

	DWORD status = 0;
	if (SUCCEEDED(m_secondaryBuffer2->GetStatus(&status)) && (status & DSBSTATUS_PLAYING) != 0)
	{
		return;
	}

	if (FAILED(m_secondaryBuffer2->SetCurrentPosition(0)) ||
		FAILED(m_secondaryBuffer2->SetVolume(DSBVOLUME_MAX)))
	{
		return;
	}

	m_secondaryBuffer2->Play(0, 0, 0);
}

void SoundClass::PlaySoundForSFX()
{
	if (!m_secondaryBuffer3)
	{
		return;
	}

	if (FAILED(m_secondaryBuffer3->SetCurrentPosition(0)) ||
		FAILED(m_secondaryBuffer3->SetVolume(-2000)))
	{
		return;
	}

	m_secondaryBuffer3->Play(0, 0, 0);
}
