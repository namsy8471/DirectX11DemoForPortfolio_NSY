////////////////////////////////////////////////////////////////////////////////
// Filename: textclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "fontclass.h"
#include "fontshaderclass.h"
#include "AlignedAllocationPolicy.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: TextClass
////////////////////////////////////////////////////////////////////////////////
class TextClass : public AlignedAllocationPolicy<16>
{
private:
	struct SentenceType
	{
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		int vertexCount, indexCount, maxLength;
		float red, green, blue;
	};

	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:
	enum class RenderType
	{
		Title,
		Stage
	};

	TextClass();
	TextClass(const TextClass&);
	~TextClass();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, HWND, int, int, XMMATRIX);
	void Shutdown();

	bool Render(ID3D11DeviceContext*, XMMATRIX, XMMATRIX);
	void ChangeRenderType(RenderType);

	bool SetFps(int, ID3D11DeviceContext*);
	bool SetCpu(int, ID3D11DeviceContext*);
	bool SetPolygons(int, ID3D11DeviceContext*);
	bool SetObject(int, ID3D11DeviceContext*);
	bool SetSizeOfScreen(ID3D11DeviceContext*);
	bool SetGoalDescription(ID3D11DeviceContext*);

private:
	bool InitializeSentence(SentenceType**, int, ID3D11Device*);
	bool UpdateSentence(SentenceType*, const char*, int, int, float, float, float, ID3D11DeviceContext*);
	void ReleaseSentence(SentenceType**);
	bool RenderSentence(ID3D11DeviceContext*, SentenceType*, XMMATRIX, XMMATRIX);

private:

	RenderType renderType = RenderType::Title;

	FontClass* m_Font;
	FontShaderClass* m_FontShader;
	int m_screenWidth, m_screenHeight;
	XMMATRIX m_baseViewMatrix;

	SentenceType* m_sentence_fps;
	SentenceType* m_sentence_cpu;

	SentenceType* m_sentence_polygons;
	SentenceType* m_sentence_objects;
	
	SentenceType* m_sentence_screen_size;
	SentenceType* m_sentence_goal_description;
};

#endif