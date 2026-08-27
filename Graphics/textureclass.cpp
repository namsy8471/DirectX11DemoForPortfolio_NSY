////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "textureclass.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

TextureClass::TextureClass()
{
}


TextureClass::~TextureClass()
{
	Shutdown();
}


bool TextureClass::Initialize(ID3D11Device* device, const WCHAR* filename)
{
	Shutdown();
	if (device == nullptr || filename == nullptr)
	{
		return false;
	}

	// Load texture data from a file by using DDS texture loader.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	if (FAILED(CreateDDSTextureFromFile(device, filename, nullptr, texture.GetAddressOf())))
	{
		return false;
	}

	m_texture.Swap(texture);
	return true;
}


void TextureClass::Shutdown()
{
	m_texture.Reset();
}


ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_texture.Get();
}
