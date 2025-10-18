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

	// ���� ���̾: ���� ǥ�� ������ ���� �����Ͽ� �����д�(�׸��� �и�) �ּ�ȭ
	float3 N = normalize(input.normal);
	float3 L = normalize(input.lightPos);
	float ndotl = saturate(dot(N, L));
	// ū �����ϼ���(����) ���̾�� ���� Ű���, �����ϼ��� �۰� ����
	float bias = max(0.00005f, 0.0003f * (1.0f - ndotl));

	// �⺻ ���(�׸���)�� ����
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// �׸��� �ؽ�ó ��ǥ ���
	float2 projectTexCoord = float2(0.0f, 0.0f);
	projectTexCoord.x =  input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

	// ��ǥ�� 0~1 ���� ���� ���� ���� ���ø�
	if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		float depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

		float lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;
		lightDepthValue = lightDepthValue - bias;

		// ����Ʈ�� �� �����ٸ� ����(���)
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
		// �׸��� �ؽ�ó ������ ����� �⺻������ ���� ó��
		lightIntensity = ndotl;
		if (lightIntensity > 0.0f)
		{
			color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

    return color;
}