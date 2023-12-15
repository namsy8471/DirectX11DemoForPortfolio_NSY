////////////////////////////////////////////////////////////////////////////////
// Filename: softshadow_ps.hlsl
////////////////////////////////////////////////////////////////////////////////


//////////////
// TEXTURES //
//////////////
Texture2D shaderTexture;
Texture2D shadowTexture;


///////////////////
// SAMPLE STATES //
///////////////////
SamplerState SampleTypeClamp;
SamplerState SampleTypeWrap;


//////////////////////
// CONSTANT BUFFERS //
//////////////////////
cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 viewPosition : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 SoftShadowPixelShader(PixelInputType input) : SV_TARGET
{
	float4 specular;

	// ��� �ȼ��� ���� �⺻ ��� ������ �ֺ� ���� ������ �����մϴ�.
    float4 color = ambientColor;

	// Initialize the specular color.
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightDir = input.lightPos;

	// �� �ȼ��� ���� ���� ����մϴ�.
	float lightIntensity = saturate(dot(input.normal, lightDir));

	if(lightIntensity > 0.0f)
	{
		// Ȯ�� ����� ���� ���� �������� ���� ������ �����մϴ�.
		color += (diffuseColor * lightIntensity);

		// ���� ���� ä��ϴ�.
		color = saturate(color);

		// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
		float3 reflection = normalize(2 * lightIntensity * input.normal - lightDir);

		// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
		specular = pow(saturate(dot(reflection, input.lightPos)), specularPower);
	}

	// �� �ؽ�ó ��ǥ ��ġ���� ���÷��� ����Ͽ� �ؽ�ó���� �ȼ� ������ ���ø��մϴ�.
	float4 textureColor = shaderTexture.Sample(SampleTypeWrap, input.tex);

	// ���� �ؽ�ó ������ �����մϴ�.
	color = color * textureColor;

	// �׸��� �ؽ�ó�� �Բ� ���� ���� �� �ؽ�ó ��ǥ�� ����մϴ�.
	float2 projectTexCoord = float2(0.0f, 0.0f);
	projectTexCoord.x =  input.viewPosition.x / input.viewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.viewPosition.y / input.viewPosition.w / 2.0f + 0.5f;

	// ���� �� �ؽ�ó ��ǥ ��ġ���� ���÷��� ����Ͽ� �׸��� �ؽ�ó���� �׸��� ���� ���ø��մϴ�.
	float shadowValue = shadowTexture.Sample(SampleTypeClamp, projectTexCoord).r;

	// �׸��ڸ� ���� ����� �����մϴ�.
	color = color * shadowValue;

	// Add the specular component last to the output color.
	color = saturate(color + specular);

    return color;
}