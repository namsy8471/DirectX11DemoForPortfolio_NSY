////////////////////////////////////////////////////////////////////////////////
// Filename: terrain_ps.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
    float3 lightDirection;
	float padding;
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 TerrainPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;


	// �� �ؽ�ó ��ǥ ��ġ���� ���÷��� ����Ͽ� �ؽ�ó���� �ȼ� ������ ���ø��մϴ�.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	// ��� �ȼ��� ���� �⺻ ��� ������ �ֺ� ���� ������ �����մϴ�.
	color = ambientColor;

	// ����� ���� �� ������ ������ŵ�ϴ�.
	lightDir = -lightDirection;

	// �� �ȼ��� ���� ���� ����մϴ�.
	lightIntensity = saturate(dot(input.normal, lightDir));

	if(lightIntensity > 0.0f)
	{
		// Ȯ�� ���� �� ������ �翡 ���� ���� Ȯ�� ���� �����մϴ�.
		color += (diffuseColor * lightIntensity);
	}

	// ���� ���� ������ ä��ϴ�.
	color = saturate(color);

	// �ؽ�ó �ȼ��� ���� ���� ���� ���Ͽ� ����� ����ϴ�.
	color = color * textureColor;

    return color;
}