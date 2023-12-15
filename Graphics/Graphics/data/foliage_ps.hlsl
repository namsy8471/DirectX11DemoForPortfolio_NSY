////////////////////////////////////////////////////////////////////////////////
// Filename: foliage_ps.hlsl
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
	float3 foliageColor : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 FoliagePixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float4 color;


	// �� �ؽ�ó ��ǥ ��ġ���� ���÷��� ����Ͽ� �ؽ�ó���� �ȼ� ������ ���ø��մϴ�.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// �ؽ�ó�� ��ǳ ���� �����մϴ�.
	color = textureColor * float4(input.foliageColor, 1.0f);

	// ���� ���� ����� ��ȭ��ŵ�ϴ�.
	color = saturate(color);

    return color;
}