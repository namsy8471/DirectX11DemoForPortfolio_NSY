///////////////////////////////////////////////////////////////////////////////
// Filename: textclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "textclass.h"


TextClass::TextClass()
{
	m_Font = 0;
	m_FontShader = 0;

	m_sentence_fps = 0;
	m_sentence_cpu = 0;
	m_sentence_objects = 0;
	m_sentence_polygons = 0;
	m_sentence_screen_size = 0;
	m_sentence_goal_description = 0;

	renderType = RenderType::Title;
}


TextClass::TextClass(const TextClass& other)
{
}


TextClass::~TextClass()
{
}


bool TextClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, 
	int screenWidth, int screenHeight, XMMATRIX baseViewMatrix)
{
	bool result;


	// Store the screen width and height.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the base view matrix.
	m_baseViewMatrix = baseViewMatrix;

	// Create the font object.
	m_Font = new FontClass;
	if(!m_Font)
	{
		return false;
	}

	// Initialize the font object.
	result = m_Font->Initialize(device, L"./data/fontdata.txt", L"./data/font.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
		return false;
	}

	// Create the font shader object.
	m_FontShader = new FontShaderClass;
	if(!m_FontShader)
	{
		return false;
	}

	// Initialize the font shader object.
	result = m_FontShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}

	result = InitializeSentence(&m_sentence_fps, 16, device);
	if(!result)
	{
		return false;
	}

	result = InitializeSentence(&m_sentence_cpu, 16, device);
	if(!result)
	{
		return false;
	}

	result = InitializeSentence(&m_sentence_polygons, 32, device);
	if (!result)
	{
		return false;
	}

	result = InitializeSentence(&m_sentence_objects, 16, device);
	if (!result)
	{
		return false;
	}

	result = InitializeSentence(&m_sentence_screen_size, 16, device);
	if (!result)
	{
		return false;
	}

	result = InitializeSentence(&m_sentence_goal_description, 64, device);
	if (!result)
	{
		return false;
	}

	return true;
}


void TextClass::Shutdown()
{
	// Release the sentence.
	ReleaseSentence(&m_sentence_fps);
	ReleaseSentence(&m_sentence_cpu);
	ReleaseSentence(&m_sentence_objects);
	ReleaseSentence(&m_sentence_polygons);
	ReleaseSentence(&m_sentence_screen_size);
	ReleaseSentence(&m_sentence_goal_description);


	// Release the font shader object.
	if(m_FontShader)
	{
		m_FontShader->Shutdown();
		delete m_FontShader;
		m_FontShader = 0;
	}

	// Release the font object.
	if(m_Font)
	{
		m_Font->Shutdown();
		delete m_Font;
		m_Font = 0;
	}

	return;
}


bool TextClass::Render(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix)
{

	switch (renderType)
	{
	case RenderType::Title:

		if (!RenderSentence(deviceContext, m_sentence_goal_description, worldMatrix, orthoMatrix))
		{
			return false;
		}
		break;

	case RenderType::Stage:

		if (!RenderSentence(deviceContext, m_sentence_fps, worldMatrix, orthoMatrix))
		{
			return false;
		}

		if (!RenderSentence(deviceContext, m_sentence_cpu, worldMatrix, orthoMatrix))
		{
			return false;
		}

		if (!RenderSentence(deviceContext, m_sentence_polygons, worldMatrix, orthoMatrix))
		{
			return false;
		}

		if (!RenderSentence(deviceContext, m_sentence_objects, worldMatrix, orthoMatrix))
		{
			return false;
		}

		if (!RenderSentence(deviceContext, m_sentence_screen_size, worldMatrix, orthoMatrix))
		{
			return false;
		}

		break;

	default:
		break;
	}

	// 디버깅용, 없으면 플리커링 일어남!
	if (!RenderSentence(deviceContext, m_sentence_fps, worldMatrix, orthoMatrix))
	{
		return false;
	}
	// 디버깅용, 없으면 플리커링 일어남!

	return true;
}

void TextClass::ChangeRenderType(RenderType type)
{
	renderType = type;
}


bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;


	// Create a new sentence object.
	*sentence = new SentenceType;
	if(!*sentence)
	{
		return false;
	}

	// Initialize the sentence buffers to null.
	(*sentence)->vertexBuffer = 0;
	(*sentence)->indexBuffer = 0;

	// Set the maximum length of the sentence.
	(*sentence)->maxLength = maxLength;

	// Set the number of vertices in the vertex array.
	(*sentence)->vertexCount = 6 * maxLength;

	// Set the number of indexes in the index array.
	(*sentence)->indexCount = (*sentence)->vertexCount;

	// Create the vertex array.
	vertices = new VertexType[(*sentence)->vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[(*sentence)->indexCount];
	if(!indices)
	{
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));

	// Initialize the index array.
	for(i=0; i<(*sentence)->indexCount; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the dynamic vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex array as it is no longer needed.
	delete [] vertices;
	vertices = 0;

	// Release the index array as it is no longer needed.
	delete [] indices;
	indices = 0;

	return true;
}


bool TextClass::UpdateSentence(SentenceType* sentence, const char* text, int positionX, int positionY, 
	float red, float green, float blue, ID3D11DeviceContext* deviceContext)
{
	int numLetters;
	VertexType* vertices;
	float drawX, drawY;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;


	// Store the color of the sentence.
	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	// Get the number of letters in the sentence.
	numLetters = (int)strlen(text);

	// Check for possible buffer overflow.
	if(numLetters > sentence->maxLength)
	{
		return false;
	}

	// Create the vertex array.
	vertices = new VertexType[sentence->vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	// Calculate the X and Y pixel position on the screen to start drawing to.
	drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
	drawY = (float)((m_screenHeight / 2) - positionY);

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	m_Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

	// Lock the vertex buffer so it can be written to.
	result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

	// Unlock the vertex buffer.
	deviceContext->Unmap(sentence->vertexBuffer, 0);

	// Release the vertex array as it is no longer needed.
	delete [] vertices;
	vertices = 0;

	return true;
}


void TextClass::ReleaseSentence(SentenceType** sentence)
{
	if(*sentence)
	{
		// Release the sentence vertex buffer.
		if((*sentence)->vertexBuffer)
		{
			(*sentence)->vertexBuffer->Release();
			(*sentence)->vertexBuffer = 0;
		}

		// Release the sentence index buffer.
		if((*sentence)->indexBuffer)
		{
			(*sentence)->indexBuffer->Release();
			(*sentence)->indexBuffer = 0;
		}

		// Release the sentence.
		delete *sentence;
		*sentence = 0;
	}

	return;
}


bool TextClass::RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, XMMATRIX worldMatrix, 
							   XMMATRIX orthoMatrix)
{
	unsigned int stride, offset;
	XMFLOAT4 pixelColor;
	bool result;

	// Set vertex buffer stride and offset.
    stride = sizeof(VertexType); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create a pixel color vector with the input sentence color.
	pixelColor = XMFLOAT4(sentence->red, sentence->green, sentence->blue, 1.0f);

	// Render the text using the font shader.
	result = m_FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, 
		orthoMatrix, m_Font->GetTexture(), pixelColor);
	if(!result)
	{
		false;
	}

	return true;
}

bool TextClass::SetFps(int fps, ID3D11DeviceContext* deviceContext)
{
	// fps를 10,000 이하로 자릅니다.
	if (fps > 9999)
	{
		fps = 9999;
	}

	// fps 정수를 문자열 형식으로 변환합니다.
	char tempString[16] = { 0, };
	_itoa_s(fps, tempString, 10);

	// fps 문자열을 설정합니다.
	char fpsString[16] = { 0, };
	strcpy_s(fpsString, "Fps: ");
	strcat_s(fpsString, tempString);

	float red = 0;
	float green = 0;
	float blue = 0;

	// fps가 60 이상이면 fps 색상을 녹색으로 설정합니다.
	if (fps >= 60)
	{
		red = 0.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// fps가 60보다 작은 경우 fps 색상을 노란색으로 설정합니다.
	if (fps < 60)
	{
		red = 1.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// fps가 30 미만이면 fps 색상을 빨간색으로 설정합니다.
	if (fps < 30)
	{
		red = 1.0f;
		green = 0.0f;
		blue = 0.0f;
	}

	// 문장 정점 버퍼를 새 문자열 정보로 업데이트합니다.
	return UpdateSentence(m_sentence_fps, fpsString, 20, 20, red, green, blue, deviceContext);
}


bool TextClass::SetCpu(int cpu, ID3D11DeviceContext* deviceContext)
{
	// cpu 정수를 문자열 형식으로 변환합니다.
	char tempString[16] = { 0, };
	_itoa_s(cpu, tempString, 10);

	// cpu 문자열을 설정합니다.
	char cpuString[16] = { 0, };
	strcpy_s(cpuString, "Cpu: ");
	strcat_s(cpuString, tempString);
	strcat_s(cpuString, "%");

	// 문장 정점 버퍼를 새 문자열 정보로 업데이트합니다.
	return UpdateSentence(m_sentence_cpu, cpuString, 20, 40, 0.0f, 1.0f, 0.0f, deviceContext);
}

bool TextClass::SetPolygons(int count, ID3D11DeviceContext* deviceContext)
{

	char tempString[32] = { 0, };
	_itoa_s(count, tempString, 10);

	char polyString[32] = { 0, };
	strcat_s(polyString, tempString);
	strcat_s(polyString, " of Polygons");

	// 문장 정점 버퍼를 새 문자열 정보로 업데이트합니다.
	return UpdateSentence(m_sentence_polygons, polyString, 20, 60, 0.0f, 1.0f, 0.0f, deviceContext);
}

bool TextClass::SetObject(int count, ID3D11DeviceContext* deviceContext)
{
	char tempString[16] = { 0, };
	_itoa_s(count, tempString, 10);

	char objString[16] = { 0, };
	strcat_s(objString, tempString);
	strcat_s(objString, " of Objects");

	// 문장 정점 버퍼를 새 문자열 정보로 업데이트합니다.
	return UpdateSentence(m_sentence_objects, objString, 20, 80, 0.0f, 1.0f, 0.0f, deviceContext);
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


	// 문장 정점 버퍼를 새 문자열 정보로 업데이트합니다.
	return UpdateSentence(m_sentence_screen_size, sizeString, 20, 100, 0.0f, 1.0f, 0.0f, deviceContext);
}


bool TextClass::SetGoalDescription(ID3D11DeviceContext* deviceContext)
{
	char goalDescriptionString[64] = {0, };
	strcat_s(goalDescriptionString, "Let's hunt all animals!");

	return UpdateSentence(m_sentence_goal_description, goalDescriptionString, 300, 200, 1.0f, 1.0f, 1.0f, deviceContext);
}