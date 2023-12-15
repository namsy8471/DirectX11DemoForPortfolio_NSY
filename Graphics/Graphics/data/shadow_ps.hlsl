////////////////////////////////////////////////////////////////////////////////
// Filename: shadow_ps.hlsl
////////////////////////////////////////////////////////////////////////////////


//////////////
// TEXTURES //
//////////////
Texture2D depthMapTexture;


///////////////////
// SAMPLE STATES //
///////////////////
SamplerState SampleTypeClamp;


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 lightViewPosition : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ShadowPixelShader(PixelInputType input) : SV_TARGET
{
	float lightIntensity;

	// 부동 소수점 정밀도 문제를 해결할 바이어스 값을 설정합니다.
	float bias = 0.001f;

	float3 lightDir = input.lightPos;

	// 기본 출력 색상을 검정 (그림자)으로 설정합니다.
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// 투영 된 텍스처 좌표를 계산합니다.
	float2 projectTexCoord = float2(0.0f, 0.0f);
	projectTexCoord.x =  input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

	// 투영 된 좌표가 0에서 1 범위에 있는지 결정합니다. 그렇다면이 픽셀은 빛의 관점에 있습니다.
	if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		// 투영 된 텍스처 좌표 위치에서 샘플러를 사용하여 깊이 텍스처에서 섀도우 맵 깊이 값을 샘플링합니다.
		float depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

		// 빛의 깊이를 계산합니다.
		float lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

		// lightDepthValue에서 바이어스를 뺍니다.
		lightDepthValue = lightDepthValue - bias;

		// 섀도우 맵 값의 깊이와 빛의 깊이를 비교하여이 픽셀을 음영 처리할지 조명할지 결정합니다.
		// 빛이 객체 앞에 있으면 픽셀을 비추고, 그렇지 않으면 객체 (오클 루더)가 그림자를 드리 우기 때문에이 픽셀을 그림자로 그립니다.
		if(lightDepthValue < depthValue)
		{
		    // 이 픽셀의 빛의 양을 계산합니다.
			lightIntensity = saturate(dot(input.normal, lightDir));

		    if(lightIntensity > 0.0f)
			{
			    color = float4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}
	else
	{
		// 이것이 그림자 맵 영역의 영역 밖에 있다면 정규 조명으로 물건을 정상적으로 그립니다.
		lightIntensity = saturate(dot(input.normal, lightDir));
		if (lightIntensity > 0.0f)
		{
			color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

    return color;
}