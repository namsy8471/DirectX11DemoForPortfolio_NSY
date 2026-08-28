#pragma once

#include <d3d11.h>

namespace D3DHelpers
{
	inline bool CreateRasterizerState(
		ID3D11Device* device,
		D3D11_CULL_MODE cullMode,
		ID3D11RasterizerState** outState,
		D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID,
		int depthBias = 0,
		float depthBiasClamp = 0.0f,
		float slopeScaledDepthBias = 0.0f)
	{
		if (device == nullptr || outState == nullptr)
		{
			return false;
		}

		D3D11_RASTERIZER_DESC description{};
		description.AntialiasedLineEnable = FALSE;
		description.CullMode = cullMode;
		description.DepthBias = depthBias;
		description.DepthBiasClamp = depthBiasClamp;
		description.DepthClipEnable = TRUE;
		description.FillMode = fillMode;
		description.FrontCounterClockwise = FALSE;
		description.MultisampleEnable = FALSE;
		description.ScissorEnable = FALSE;
		description.SlopeScaledDepthBias = slopeScaledDepthBias;
		return SUCCEEDED(device->CreateRasterizerState(&description, outState));
	}

	inline bool CreateDepthStencilState(
		ID3D11Device* device,
		ID3D11DepthStencilState** outState,
		BOOL depthEnable = TRUE,
		D3D11_DEPTH_WRITE_MASK depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL)
	{
		if (device == nullptr || outState == nullptr)
		{
			return false;
		}

		D3D11_DEPTH_STENCIL_DESC description{};
		description.DepthEnable = depthEnable;
		description.DepthWriteMask = depthWriteMask;
		description.DepthFunc = D3D11_COMPARISON_LESS;
		description.StencilEnable = TRUE;
		description.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		description.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		description.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		description.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		description.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		description.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		return SUCCEEDED(device->CreateDepthStencilState(&description, outState));
	}

	inline bool CreateBlendState(
		ID3D11Device* device,
		ID3D11BlendState** outState,
		BOOL blendEnable = TRUE)
	{
		if (device == nullptr || outState == nullptr)
		{
			return false;
		}

		D3D11_BLEND_DESC description{};
		description.RenderTarget[0].BlendEnable = blendEnable;
		description.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		description.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		description.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		description.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		description.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		description.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		description.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		return SUCCEEDED(device->CreateBlendState(&description, outState));
	}
}
