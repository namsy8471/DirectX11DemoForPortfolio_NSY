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
	// 각 이웃 픽셀이 흐림에 기여할 가중치를 만듭니다.
	float weight0 = 1.0f;
	float weight1 = 0.9f;
	float weight2 = 0.55f;
	float weight3 = 0.18f;
	float weight4 = 0.1f;

	// 가중치를 평균화하는 정규화 된 값을 만듭니다.
	float normalization = (weight0 + 2.0f * (weight1 + weight2 + weight3 + weight4));

	// 가중치를 평균화 합니다.
	weight0 = weight0 / normalization;
	weight1 = weight1 / normalization;
	weight2 = weight2 / normalization;
	weight3 = weight3 / normalization;
	weight4 = weight4 / normalization;

	// 검은 색으로 초기화합니다.
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 각각의 특정 가중치로 9 개의 수평 픽셀을 색상에 추가합니다.
	color += shaderTexture.Sample(SampleType, input.texCoord1) * weight4;
	color += shaderTexture.Sample(SampleType, input.texCoord2) * weight3;
	color += shaderTexture.Sample(SampleType, input.texCoord3) * weight2;
	color += shaderTexture.Sample(SampleType, input.texCoord4) * weight1;
	color += shaderTexture.Sample(SampleType, input.texCoord5) * weight0;
	color += shaderTexture.Sample(SampleType, input.texCoord6) * weight1;
	color += shaderTexture.Sample(SampleType, input.texCoord7) * weight2;
	color += shaderTexture.Sample(SampleType, input.texCoord8) * weight3;
	color += shaderTexture.Sample(SampleType, input.texCoord9) * weight4;

	// 알파 채널을 1로 설정합니다.
	color.a = 1.0f;

    return color;
}