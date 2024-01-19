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
	

	// ������ ��ġ�� ������ �ȼ����� ���� �ٸ� ���� ���� ����մϴ�.
	lightIntensity1 = saturate(dot(input.normal, input.lightPos1));
	lightIntensity2 = saturate(dot(input.normal, input.lightPos2));
	lightIntensity3 = saturate(dot(input.normal, input.lightPos3));
	lightIntensity4 = saturate(dot(input.normal, input.lightPos4));
	
	// �� ���� ǥ�� �� ������ Ȯ�� ���� ���� �����մϴ�.
	color1 = diffuseColor[0] * lightIntensity1;
	color2 = diffuseColor[1] * lightIntensity2;
	color3 = diffuseColor[2] * lightIntensity3;
	color4 = diffuseColor[3] * lightIntensity4;

	// �� ��ġ���� �ؽ�ó �ȼ��� ���ø��մϴ�.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// ���� ����� �������� �ؽ�ó �ȼ��� �� ���� ���� ������ �������� ���մϴ�.
	color = saturate(color1 + color2 + color3 + color4) * textureColor;
	
	return color;
}
