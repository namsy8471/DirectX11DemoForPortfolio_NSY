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

	// 모든 픽셀에 대해 기본 출력 색상을 주변 광원 값으로 설정합니다.
    float4 color = ambientColor;

	// Initialize the specular color.
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightDir = input.lightPos;

	// 이 픽셀의 빛의 양을 계산합니다.
	float lightIntensity = saturate(dot(input.normal, lightDir));

	if(lightIntensity > 0.0f)
	{
		// 확산 색상과 빛의 양을 기준으로 빛의 색상을 결정합니다.
		color += (diffuseColor * lightIntensity);

		// 밝은 색을 채웁니다.
		color = saturate(color);

		// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
		float3 reflection = normalize(2 * lightIntensity * input.normal - lightDir);

		// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
		specular = pow(saturate(dot(reflection, input.lightPos)), specularPower);
	}

	// 이 텍스처 좌표 위치에서 샘플러를 사용하여 텍스처에서 픽셀 색상을 샘플링합니다.
	float4 textureColor = shaderTexture.Sample(SampleTypeWrap, input.tex);

	// 빛과 텍스처 색상을 결합합니다.
	color = color * textureColor;

	// 그림자 텍스처와 함께 사용될 투영 된 텍스처 좌표를 계산합니다.
	float2 projectTexCoord = float2(0.0f, 0.0f);
	projectTexCoord.x =  input.viewPosition.x / input.viewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.viewPosition.y / input.viewPosition.w / 2.0f + 0.5f;

	// 투영 된 텍스처 좌표 위치에서 샘플러를 사용하여 그림자 텍스처에서 그림자 값을 샘플링합니다.
	float shadowValue = shadowTexture.Sample(SampleTypeClamp, projectTexCoord).r;

	// 그림자를 최종 색상과 결합합니다.
	color = color * shadowValue;

	// Add the specular component last to the output color.
	color = saturate(color + specular);

    return color;
}