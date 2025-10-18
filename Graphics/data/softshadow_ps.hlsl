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
	float3 lightDirection; // view-space light direction
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
	float3 normal : NORMAL;          // view-space normal
    float4 viewPosition : TEXCOORD1; // clip-space position for screen-space sampling
	float3 lightPos : TEXCOORD2;     // view-space light direction
	float3 viewDir : TEXCOORD3;      // view-space view direction
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 SoftShadowPixelShader(PixelInputType input) : SV_TARGET
{
	// Normalize interpolated vectors
	float3 N = normalize(input.normal);
	float3 L = normalize(input.lightPos);
	float3 V = normalize(input.viewDir);

	// Ambient base
    float4 color = ambientColor;

	// Diffuse
	float NdotL = saturate(dot(N, L));
	if (NdotL > 0.0f)
	{
		color += diffuseColor * NdotL;
	}

	// Sample base texture
	float4 textureColor = shaderTexture.Sample(SampleTypeWrap, input.tex);
	color *= textureColor;

	// Projective coords for shadow texture (we passed clip-space position as viewPosition)
	float2 projectTexCoord;
	projectTexCoord.x =  input.viewPosition.x / input.viewPosition.w * 0.5f + 0.5f;
	projectTexCoord.y = -input.viewPosition.y / input.viewPosition.w * 0.5f + 0.5f;

	// Sample blurred shadow
	float shadowValue = shadowTexture.Sample(SampleTypeClamp, projectTexCoord).r;
	color *= shadowValue;

	// Specular (Blinn-Phong)
	float3 H = normalize(L + V);
	float spec = pow(saturate(dot(N, H)), specularPower);
	float4 specular = spec * specularColor;
	color = saturate(color + specular);

    return color;
}