////////////////////////////////////////////////////////////////////////////////
// Filename: DXGIHelpers.h
// Helper utilities for DXGI initialization
////////////////////////////////////////////////////////////////////////////////
#ifndef _DXGIHELPERS_H_
#define _DXGIHELPERS_H_

#include <dxgi.h>
#include <vector>

namespace DXGIHelpers
{
	// DXGI 팩토리 및 어댑터 정보 구조체
	struct AdapterInfo
	{
		int videoCardMemory;
		char videoCardDescription[128];
		unsigned int refreshRateNumerator;
		unsigned int refreshRateDenominator;
	};

	// DXGI 어댑터 정보 수집
	inline bool GetAdapterInfo(int screenWidth, int screenHeight, AdapterInfo& outInfo)
	{
		IDXGIFactory* factory = nullptr;
		IDXGIAdapter* adapter = nullptr;
		IDXGIOutput* adapterOutput = nullptr;
		DXGI_MODE_DESC* displayModeList = nullptr;
		unsigned int numModes = 0;
		DXGI_ADAPTER_DESC adapterDesc;
		size_t stringLength = 0;

		// Create factory
		if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory))))
		{
			return false;
		}

		auto cleanup = [&]() {
			if (displayModeList) delete[] displayModeList;
			if (adapterOutput) adapterOutput->Release();
			if (adapter) adapter->Release();
			if (factory) factory->Release();
		};

		// Get adapter
		if (FAILED(factory->EnumAdapters(0, &adapter)))
		{
			cleanup();
			return false;
		}

		// Get adapter output
		if (FAILED(adapter->EnumOutputs(0, &adapterOutput)))
		{
			cleanup();
			return false;
		}

		// Get display mode list
		if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr)) || numModes == 0)
		{
			cleanup();
			return false;
		}

		displayModeList = new DXGI_MODE_DESC[numModes];
		if (!displayModeList)
		{
			cleanup();
			return false;
		}

		if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList)))
		{
			cleanup();
			return false;
		}

		// Find matching display mode
		outInfo.refreshRateNumerator = 0;
		outInfo.refreshRateDenominator = 1;
		for (unsigned int i = 0; i < numModes; i++)
		{
			if (displayModeList[i].Width == static_cast<unsigned int>(screenWidth) &&
				displayModeList[i].Height == static_cast<unsigned int>(screenHeight))
			{
				outInfo.refreshRateNumerator = displayModeList[i].RefreshRate.Numerator;
				outInfo.refreshRateDenominator = displayModeList[i].RefreshRate.Denominator;
				break;
			}
		}

		// Get adapter description
		if (FAILED(adapter->GetDesc(&adapterDesc)))
		{
			cleanup();
			return false;
		}

		outInfo.videoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

		if (wcstombs_s(&stringLength, outInfo.videoCardDescription, sizeof(outInfo.videoCardDescription), 
					   adapterDesc.Description, sizeof(outInfo.videoCardDescription)) != 0)
		{
			cleanup();
			return false;
		}

		cleanup();
		return true;
	}
}

#endif
