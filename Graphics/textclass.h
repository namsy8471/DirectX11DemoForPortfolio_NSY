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

#include <memory>
#include <wrl/client.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: TextClass
////////////////////////////////////////////////////////////////////////////////
class TextClass : public AlignedAllocationPolicy<16>
{
private:
	struct SentenceType
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
		int vertexCount = 0;
		int indexCount = 0;
		int maxLength = 0;
		float red = 0.0f;
		float green = 0.0f;
		float blue = 0.0f;
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
	TextClass(const TextClass&) = delete;
	TextClass& operator=(const TextClass&) = delete;
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
	bool InitializeSentence(std::unique_ptr<SentenceType>&, int, ID3D11Device*);
	bool UpdateSentence(SentenceType*, const char*, int, int, float, float, float, ID3D11DeviceContext*);
	void ReleaseSentence(std::unique_ptr<SentenceType>&);
	bool RenderSentence(ID3D11DeviceContext*, SentenceType*, XMMATRIX, XMMATRIX);

private:

	RenderType renderType = RenderType::Title;

	std::unique_ptr<FontClass> m_Font;
	std::unique_ptr<FontShaderClass> m_FontShader;
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	XMMATRIX m_baseViewMatrix = XMMatrixIdentity();

	std::unique_ptr<SentenceType> m_sentence_fps;
	std::unique_ptr<SentenceType> m_sentence_cpu;

	std::unique_ptr<SentenceType> m_sentence_polygons;
	std::unique_ptr<SentenceType> m_sentence_objects;
	
	std::unique_ptr<SentenceType> m_sentence_screen_size;
	std::unique_ptr<SentenceType> m_sentence_goal_description;
};

#endif
