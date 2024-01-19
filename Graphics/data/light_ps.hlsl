////////////////////////////////////////////////////////////////////////////////
// Filename: light_ps.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// DEFINES //
/////////////
#define NUM_LIGHTS 4


/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightColorBuffer
{
	float4 diffuseColor[NUM_LIGHTS];
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 lightPos1 : TEXCOORD1;
	float3 lightPos2 : TEXCOORD2;
	float3 lightPos3 : TEXCOORD3;
	float3 lightPos4 : TEXCOORD4;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float lightIntensity1, lightIntensity2, lightIntensity3, lightIntensity4;
	float4 color, color1, color2, color3, color4;
	

	// 조명의 위치에 따라이 픽셀에서 서로 다른 빛의 양을 계산합니다.
	lightIntensity1 = saturate(dot(input.normal, input.lightPos1));
	lightIntensity2 = saturate(dot(input.normal, input.lightPos2));
	lightIntensity3 = saturate(dot(input.normal, input.lightPos3));
	lightIntensity4 = saturate(dot(input.normal, input.lightPos4));
	
	// 네 개의 표시 등 각각의 확산 색상 양을 결정합니다.
	color1 = diffuseColor[0] * lightIntensity1;
	color2 = diffuseColor[1] * lightIntensity2;
	color3 = diffuseColor[2] * lightIntensity3;
	color4 = diffuseColor[3] * lightIntensity4;

	// 이 위치에서 텍스처 픽셀을 샘플링합니다.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// 최종 결과를 얻으려면 텍스처 픽셀을 네 가지 밝은 색상의 조합으로 곱합니다.
	color = saturate(color1 + color2 + color3 + color4) * textureColor;
	
	return color;
}
