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


	// 이 텍스처 좌표 위치에서 샘플러를 사용하여 텍스처에서 픽셀 색상을 샘플링합니다.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// 텍스처와 단풍 색을 결합합니다.
	color = textureColor * float4(input.foliageColor, 1.0f);

	// 최종 색상 결과를 포화시킵니다.
	color = saturate(color);

    return color;
}