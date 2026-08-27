///////////////////////////////////////////////////////////////////////////////
// Filename: textclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "textclass.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>


TextClass::TextClass()
{
}


TextClass::~TextClass()
{
	Shutdown();
}


bool TextClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, 
	int screenWidth, int screenHeight, XMMATRIX baseViewMatrix)
{
	Shutdown();
	if (device == nullptr || deviceContext == nullptr || screenWidth <= 0 || screenHeight <= 0)
	{
		return false;
	}

	// Store the screen width and height.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the base view matrix.
	m_baseViewMatrix = baseViewMatrix;

	auto font = std::make_unique<FontClass>();
	if(!font->Initialize(device, L"./data/fontdata.txt", L"./data/font.dds"))
	{
		MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
		return false;
	}

	auto fontShader = std::make_unique<FontShaderClass>();
	if(!fontShader->Initialize(device, hwnd))
	{
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}

	std::unique_ptr<SentenceType> sentenceFps;
	std::unique_ptr<SentenceType> sentenceCpu;
	std::unique_ptr<SentenceType> sentencePolygons;
	std::unique_ptr<SentenceType> sentenceObjects;
	std::unique_ptr<SentenceType> sentenceScreenSize;
	std::unique_ptr<SentenceType> sentenceGoalDescription;

	if(!InitializeSentence(sentenceFps, 16, device))
	{
		return false;
	}

	if(!InitializeSentence(sentenceCpu, 16, device))
	{
		return false;
	}

	if (!InitializeSentence(sentencePolygons, 32, device))
	{
		return false;
	}

	if (!InitializeSentence(sentenceObjects, 16, device))
	{
		return false;
	}

	if (!InitializeSentence(sentenceScreenSize, 16, device))
	{
		return false;
	}

	if (!InitializeSentence(sentenceGoalDescription, 64, device))
	{
		return false;
	}

	m_Font = std::move(font);
	m_FontShader = std::move(fontShader);
	m_sentence_fps = std::move(sentenceFps);
	m_sentence_cpu = std::move(sentenceCpu);
	m_sentence_polygons = std::move(sentencePolygons);
	m_sentence_objects = std::move(sentenceObjects);
	m_sentence_screen_size = std::move(sentenceScreenSize);
	m_sentence_goal_description = std::move(sentenceGoalDescription);

	return true;
}


void TextClass::Shutdown()
{
	ReleaseSentence(m_sentence_fps);
	ReleaseSentence(m_sentence_cpu);
	ReleaseSentence(m_sentence_objects);
	ReleaseSentence(m_sentence_polygons);
	ReleaseSentence(m_sentence_screen_size);
	ReleaseSentence(m_sentence_goal_description);
	m_FontShader.reset();
	m_Font.reset();
	m_screenWidth = 0;
	m_screenHeight = 0;
}


bool TextClass::Render(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix)
{

	switch (renderType)
	{
	case RenderType::Title:

		if (!RenderSentence(deviceContext, m_sentence_goal_description.get(), worldMatrix, orthoMatrix))
		{
			return false;
		}
		break;

	case RenderType::Stage:

		if (!RenderSentence(deviceContext, m_sentence_fps.get(), worldMatrix, orthoMatrix))
		{
			return false;
		}

		if (!RenderSentence(deviceContext, m_sentence_cpu.get(), worldMatrix, orthoMatrix))
		{
			return false;
		}

		if (!RenderSentence(deviceContext, m_sentence_polygons.get(), worldMatrix, orthoMatrix))
		{
			return false;
		}

		if (!RenderSentence(deviceContext, m_sentence_objects.get(), worldMatrix, orthoMatrix))
		{
			return false;
		}

		if (!RenderSentence(deviceContext, m_sentence_screen_size.get(), worldMatrix, orthoMatrix))
		{
			return false;
		}

		break;

	default:
		break;
	}

	// ������, ������ �ø�Ŀ�� �Ͼ!
	if (!RenderSentence(deviceContext, m_sentence_fps.get(), worldMatrix, orthoMatrix))
	{
		return false;
	}
	// ������, ������ �ø�Ŀ�� �Ͼ!

	return true;
}

void TextClass::ChangeRenderType(RenderType type)
{
	renderType = type;
}


bool TextClass::InitializeSentence(std::unique_ptr<SentenceType>& sentence, int maxLength, ID3D11Device* device)
{
	if (device == nullptr || maxLength <= 0)
	{
		return false;
	}

	auto newSentence = std::make_unique<SentenceType>();

	// Set the maximum length of the sentence.
	newSentence->maxLength = maxLength;

	// Set the number of vertices in the vertex array.
	newSentence->vertexCount = 6 * maxLength;

	// Set the number of indexes in the index array.
	newSentence->indexCount = newSentence->vertexCount;

	std::vector<VertexType> vertices(static_cast<std::size_t>(newSentence->vertexCount));
	std::vector<std::uint32_t> indices(static_cast<std::size_t>(newSentence->indexCount));

	// Initialize the index array.
	for(int i=0; i<newSentence->indexCount; i++)
	{
		indices[static_cast<std::size_t>(i)] = static_cast<std::uint32_t>(i);
	}

	// Set up the description of the dynamic vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc{};
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * newSentence->vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData{};
    vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	if(FAILED(device->CreateBuffer(
		&vertexBufferDesc, &vertexData, newSentence->vertexBuffer.GetAddressOf())))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc{};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(std::uint32_t) * newSentence->indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	D3D11_SUBRESOURCE_DATA indexData{};
    indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	if(FAILED(device->CreateBuffer(
		&indexBufferDesc, &indexData, newSentence->indexBuffer.GetAddressOf())))
	{
		return false;
	}

	sentence = std::move(newSentence);
	return true;
}


bool TextClass::UpdateSentence(SentenceType* sentence, const char* text, int positionX, int positionY, 
	float red, float green, float blue, ID3D11DeviceContext* deviceContext)
{
	if (sentence == nullptr || text == nullptr || deviceContext == nullptr || m_Font == nullptr ||
		sentence->vertexBuffer == nullptr)
	{
		return false;
	}

	// Store the color of the sentence.
	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	// Get the number of letters in the sentence.
	const int numLetters = static_cast<int>(std::strlen(text));

	// Check for possible buffer overflow.
	if(numLetters > sentence->maxLength)
	{
		return false;
	}

	std::vector<VertexType> vertices(static_cast<std::size_t>(sentence->vertexCount));

	// Calculate the X and Y pixel position on the screen to start drawing to.
	const float drawX = static_cast<float>(((m_screenWidth / 2) * -1) + positionX);
	const float drawY = static_cast<float>((m_screenHeight / 2) - positionY);

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	m_Font->BuildVertexArray(vertices.data(), text, drawX, drawY);

	// Lock the vertex buffer so it can be written to.
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	if(FAILED(deviceContext->Map(
		sentence->vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	auto* verticesPtr = static_cast<VertexType*>(mappedResource.pData);

	// Copy the data into the vertex buffer.
	std::memcpy(verticesPtr, vertices.data(), sizeof(VertexType) * sentence->vertexCount);

	// Unlock the vertex buffer.
	deviceContext->Unmap(sentence->vertexBuffer.Get(), 0);

	return true;
}


void TextClass::ReleaseSentence(std::unique_ptr<SentenceType>& sentence)
{
	sentence.reset();
}


bool TextClass::RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, XMMATRIX worldMatrix, 
							   XMMATRIX orthoMatrix)
{
	if (deviceContext == nullptr || sentence == nullptr || m_FontShader == nullptr || m_Font == nullptr ||
		sentence->vertexBuffer == nullptr || sentence->indexBuffer == nullptr)
	{
		return false;
	}

	unsigned int stride, offset;
	XMFLOAT4 pixelColor;
	bool result;

	// Set vertex buffer stride and offset.
    stride = sizeof(VertexType); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	ID3D11Buffer* vertexBuffer = sentence->vertexBuffer.Get();
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(sentence->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create a pixel color vector with the input sentence color.
	pixelColor = XMFLOAT4(sentence->red, sentence->green, sentence->blue, 1.0f);

	// Render the text using the font shader.
	result = m_FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, 
		orthoMatrix, m_Font->GetTexture(), pixelColor);
	if(!result)
	{
		return false;
	}

	return true;
}

bool TextClass::SetFps(int fps, ID3D11DeviceContext* deviceContext)
{
	// fps�� 10,000 ���Ϸ� �ڸ��ϴ�.
	if (fps > 9999)
	{
		fps = 9999;
	}

	// fps ������ ���ڿ� �������� ��ȯ�մϴ�.
	char tempString[16] = { 0, };
	_itoa_s(fps, tempString, 10);

	// fps ���ڿ��� �����մϴ�.
	char fpsString[16] = { 0, };
	strcpy_s(fpsString, "Fps: ");
	strcat_s(fpsString, tempString);

	float red = 0;
	float green = 0;
	float blue = 0;

	// fps�� 60 �̻��̸� fps ������ ������� �����մϴ�.
	if (fps >= 60)
	{
		red = 0.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// fps�� 60���� ���� ��� fps ������ ��������� �����մϴ�.
	if (fps < 60)
	{
		red = 1.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// fps�� 30 �̸��̸� fps ������ ���������� �����մϴ�.
	if (fps < 30)
	{
		red = 1.0f;
		green = 0.0f;
		blue = 0.0f;
	}

	// ���� ���� ���۸� �� ���ڿ� ������ ������Ʈ�մϴ�.
	return UpdateSentence(m_sentence_fps.get(), fpsString, 20, 20, red, green, blue, deviceContext);
}


bool TextClass::SetCpu(int cpu, ID3D11DeviceContext* deviceContext)
{
	// cpu ������ ���ڿ� �������� ��ȯ�մϴ�.
	char tempString[16] = { 0, };
	_itoa_s(cpu, tempString, 10);

	// cpu ���ڿ��� �����մϴ�.
	char cpuString[16] = { 0, };
	strcpy_s(cpuString, "Cpu: ");
	strcat_s(cpuString, tempString);
	strcat_s(cpuString, "%");

	// ���� ���� ���۸� �� ���ڿ� ������ ������Ʈ�մϴ�.
	return UpdateSentence(m_sentence_cpu.get(), cpuString, 20, 40, 0.0f, 1.0f, 0.0f, deviceContext);
}

bool TextClass::SetPolygons(int count, ID3D11DeviceContext* deviceContext)
{

	char tempString[32] = { 0, };
	_itoa_s(count, tempString, 10);

	char polyString[32] = { 0, };
	strcat_s(polyString, tempString);
	strcat_s(polyString, " of Polygons");

	// ���� ���� ���۸� �� ���ڿ� ������ ������Ʈ�մϴ�.
	return UpdateSentence(m_sentence_polygons.get(), polyString, 20, 60, 0.0f, 1.0f, 0.0f, deviceContext);
}

bool TextClass::SetObject(int count, ID3D11DeviceContext* deviceContext)
{
	char tempString[16] = { 0, };
	_itoa_s(count, tempString, 10);

	char objString[16] = { 0, };
	strcat_s(objString, tempString);
	strcat_s(objString, " of Objects");

	// ���� ���� ���۸� �� ���ڿ� ������ ������Ʈ�մϴ�.
	return UpdateSentence(m_sentence_objects.get(), objString, 20, 80, 0.0f, 1.0f, 0.0f, deviceContext);
}

bool TextClass::SetSizeOfScreen(ID3D11DeviceContext* deviceContext)
{
	char tempString[16] = { 0, };
	_itoa_s(m_screenWidth, tempString, 10);
	char tempString2[16] = { 0, };
	_itoa_s(m_screenHeight, tempString2, 10);

	char sizeString[16] = { 0, };
	strcat_s(sizeString, tempString);
	strcat_s(sizeString, "x");
	strcat_s(sizeString, tempString2);


	// ���� ���� ���۸� �� ���ڿ� ������ ������Ʈ�մϴ�.
	return UpdateSentence(m_sentence_screen_size.get(), sizeString, 20, 100, 0.0f, 1.0f, 0.0f, deviceContext);
}


bool TextClass::SetGoalDescription(ID3D11DeviceContext* deviceContext)
{
	char goalDescriptionString[64] = {0, };
	strcat_s(goalDescriptionString, "Let's hunt all animals!");

	return UpdateSentence(m_sentence_goal_description.get(), goalDescriptionString, 300, 200, 1.0f, 1.0f, 1.0f, deviceContext);
}
