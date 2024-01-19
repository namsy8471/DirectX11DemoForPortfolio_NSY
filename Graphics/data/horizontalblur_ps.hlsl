////////////////////////////////////////////////////////////////////////////////
// Filename: horizontalblur_ps.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture;
SamplerState SampleType;


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float2 texCoord1 : TEXCOORD1;
	float2 texCoord2 : TEXCOORD2;
	float2 texCoord3 : TEXCOORD3;
	float2 texCoord4 : TEXCOORD4;
	float2 texCoord5 : TEXCOORD5;
	float2 texCoord6 : TEXCOORD6;
	float2 texCoord7 : TEXCOORD7;
	float2 texCoord8 : TEXCOORD8;
	float2 texCoord9 : TEXCOORD9;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 HorizontalBlurPixelShader(PixelInputType input) : SV_TARGET
{
	// �� �̿� �ȼ��� �帲�� �⿩�� ����ġ�� ����ϴ�.
	float weight0 = 1.0f;
	float weight1 = 0.9f;
	float weight2 = 0.55f;
	float weight3 = 0.18f;
	float weight4 = 0.1f;

	// ����ġ�� ���ȭ�ϴ� ����ȭ �� ���� ����ϴ�.
	float normalization = (weight0 + 2.0f * (weight1 + weight2 + weight3 + weight4));

	// ����ġ�� ���ȭ �մϴ�.
	weight0 = weight0 / normalization;
	weight1 = weight1 / normalization;
	weight2 = weight2 / normalization;
	weight3 = weight3 / normalization;
	weight4 = weight4 / normalization;

	// ���� ������ �ʱ�ȭ�մϴ�.
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// ������ Ư�� ����ġ�� 9 ���� ���� �ȼ��� ���� �߰��մϴ�.
	color += shaderTexture.Sample(SampleType, input.texCoord1) * weight4;
	color += shaderTexture.Sample(SampleType, input.texCoord2) * weight3;
	color += shaderTexture.Sample(SampleType, input.texCoord3) * weight2;
	color += shaderTexture.Sample(SampleType, input.texCoord4) * weight1;
	color += shaderTexture.Sample(SampleType, input.texCoord5) * weight0;
	color += shaderTexture.Sample(SampleType, input.texCoord6) * weight1;
	color += shaderTexture.Sample(SampleType, input.texCoord7) * weight2;
	color += shaderTexture.Sample(SampleType, input.texCoord8) * weight3;
	color += shaderTexture.Sample(SampleType, input.texCoord9) * weight4;

	// ���� ä���� 1�� �����մϴ�.
	color.a = 1.0f;

    return color;
}