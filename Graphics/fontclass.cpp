///////////////////////////////////////////////////////////////////////////////
// Filename: fontclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "fontclass.h"

#include <cstring>
#include <utility>


FontClass::FontClass()
{
}


FontClass::~FontClass()
{
	Shutdown();
}


bool FontClass::Initialize(ID3D11Device* device, const WCHAR* fontFilename, const WCHAR* textureFilename)
{
	Shutdown();
	if (device == nullptr || fontFilename == nullptr || textureFilename == nullptr)
	{
		return false;
	}

	bool result;


	// Load in the text file containing the font data.
	result = LoadFontData(fontFilename);
	if(!result)
	{
		Shutdown();
		return false;
	}

	// Load the texture that has the font characters on it.
	result = LoadTexture(device, textureFilename);
	if(!result)
	{
		Shutdown();
		return false;
	}

	return true;
}


void FontClass::Shutdown()
{
	// Release the font texture.
	ReleaseTexture();

	// Release the font data.
	ReleaseFontData();

	return;
}


bool FontClass::LoadFontData(const WCHAR* filename)
{
	ifstream fin;
	char temp = 0;

	// Read in the font size and spacing between chars.
	fin.open(filename);
	if(fin.fail())
	{
		return false;
	}
	std::vector<FontType> font(95u);

	// Read in the 95 used ascii characters for text.
	for(int i = 0; i < 95; ++i)
	{
		while (fin.get(temp) && temp != ' ') {}
		if (!fin)
		{
			return false;
		}
		while (fin.get(temp) && temp != ' ') {}
		if (!fin)
		{
			return false;
		}

		FontType& value = font[static_cast<std::size_t>(i)];
		if (!(fin >> value.left >> value.right >> value.size))
		{
			return false;
		}
	}

	m_Font = std::move(font);
	return true;
}


void FontClass::ReleaseFontData()
{
	m_Font.clear();
}


bool FontClass::LoadTexture(ID3D11Device* device, const WCHAR* filename)
{
	auto texture = std::make_unique<TextureClass>();
	if(!texture->Initialize(device, filename))
	{
		return false;
	}

	m_Texture = std::move(texture);
	return true;
}


void FontClass::ReleaseTexture()
{
	m_Texture.reset();
}


ID3D11ShaderResourceView* FontClass::GetTexture()
{
	return m_Texture ? m_Texture->GetTexture() : nullptr;
}


void FontClass::BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY)
{
	if (vertices == nullptr || sentence == nullptr || m_Font.size() != 95u)
	{
		return;
	}

	auto* vertexPtr = static_cast<VertexType*>(vertices);
	const int numLetters = static_cast<int>(strlen(sentence));
	int index = 0;

	// Draw each letter onto a quad.
	for(int i=0; i<numLetters; i++)
	{
		const int letter = static_cast<int>(static_cast<unsigned char>(sentence[i])) - 32;

		// If the letter is a space then just move over three pixels.
		if(letter == 0)
		{
			drawX = drawX + 3.0f;
		}
		else if (letter > 0 && letter < static_cast<int>(m_Font.size()))
		{
			// First triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX, (drawY - 16), 0.0f);  // Bottom left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 1.0f);
			index++;

			// Second triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX + m_Font[letter].size, drawY, 0.0f);  // Top right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			// Update the x location for drawing by the size of the letter and one pixel.
			drawX = drawX + m_Font[letter].size + 1.0f;
		}
	}

	return;
}
