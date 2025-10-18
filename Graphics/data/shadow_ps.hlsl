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

	// 동적 바이어스: 빛과 표면 각도에 따라 조정하여 피터패닝(그림자 분리) 최소화
	float3 N = normalize(input.normal);
	float3 L = normalize(input.lightPos);
	float ndotl = saturate(dot(N, L));
	// 큰 각도일수록(접선) 바이어스를 조금 키우고, 정면일수록 작게 유지
	float bias = max(0.00005f, 0.0003f * (1.0f - ndotl));

	// 기본 출력(그림자)은 검정
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// 그림자 텍스처 좌표 계산
	float2 projectTexCoord = float2(0.0f, 0.0f);
	projectTexCoord.x =  input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

	// 좌표가 0~1 범위 내에 있을 때만 샘플링
	if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		float depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

		float lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;
		lightDepthValue = lightDepthValue - bias;

		// 라이트에 더 가깝다면 조명(흰색)
		if(lightDepthValue < depthValue)
		{
			lightIntensity = ndotl;
			if(lightIntensity > 0.0f)
			{
				color = float4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}
	else
	{
		// 그림자 텍스처 범위를 벗어나면 기본적으로 조명 처리
		lightIntensity = ndotl;
		if (lightIntensity > 0.0f)
		{
			color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

    return color;
}