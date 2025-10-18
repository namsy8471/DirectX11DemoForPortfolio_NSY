////////////////////////////////////////////////////////////////////////////////
// Filename: D3DHelpers.h
// Helper utilities for reducing D3D11 code duplication
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DHELPERS_H_
#define _D3DHELPERS_H_

#include <d3d11.h>
#include <functional>

namespace D3DHelpers
{
	// COM 객체 안전 해제 템플릿
	template<typename T>
	inline void SafeRelease(T*& ptr)
	{
		if (ptr)
		{
			ptr->Release();
			ptr = nullptr;
		}
	}

	// HRESULT 체크 및 정리 헬퍼
	inline bool CheckResult(HRESULT result, const std::function<void()>& cleanup = nullptr)
	{
		if (FAILED(result))
		{
			if (cleanup)
			{
				cleanup();
			}
			return false;
		}
		return true;
	}

	// 래스터라이저 상태 생성 헬퍼
	inline bool CreateRasterizerState(
		ID3D11Device* device,
		D3D11_CULL_MODE cullMode,
		ID3D11RasterizerState** outState,
		D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID,
		int depthBias = 0,
		float depthBiasClamp = 0.0f,
		float slopeScaledDepthBias = 0.0f)
	{
		D3D11_RASTERIZER_DESC desc;
		desc.AntialiasedLineEnable = FALSE;
		desc.CullMode = cullMode;
		desc.DepthBias = depthBias;
		desc.DepthBiasClamp = depthBiasClamp;
		desc.DepthClipEnable = TRUE;
		desc.FillMode = fillMode;
		desc.FrontCounterClockwise = FALSE;
		desc.MultisampleEnable = FALSE;
		desc.ScissorEnable = FALSE;
		desc.SlopeScaledDepthBias = slopeScaledDepthBias;

		return SUCCEEDED(device->CreateRasterizerState(&desc, outState));
	}

	// 깊이 스텐실 상태 생성 헬퍼
	inline bool CreateDepthStencilState(
		ID3D11Device* device,
		ID3D11DepthStencilState** outState,
		BOOL depthEnable = TRUE,
		D3D11_DEPTH_WRITE_MASK depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL)
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.DepthEnable = depthEnable;
		desc.DepthWriteMask = depthWriteMask;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.StencilEnable = TRUE;
		desc.StencilReadMask = 0xFF;
		desc.StencilWriteMask = 0xFF;

		// Front-facing stencil operations
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Back-facing stencil operations
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		return SUCCEEDED(device->CreateDepthStencilState(&desc, outState));
	}

	// 블렌드 상태 생성 헬퍼
	inline bool CreateBlendState(
		ID3D11Device* device,
		ID3D11BlendState** outState,
		BOOL blendEnable = TRUE)
	{
		D3D11_BLEND_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.RenderTarget[0].BlendEnable = blendEnable;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

		return SUCCEEDED(device->CreateBlendState(&desc, outState));
	}
}

#endif
