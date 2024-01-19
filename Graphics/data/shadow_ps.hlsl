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

	// �ε� �Ҽ��� ���е� ������ �ذ��� ���̾ ���� �����մϴ�.
	float bias = 0.001f;

	float3 lightDir = input.lightPos;

	// �⺻ ��� ������ ���� (�׸���)���� �����մϴ�.
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// ���� �� �ؽ�ó ��ǥ�� ����մϴ�.
	float2 projectTexCoord = float2(0.0f, 0.0f);
	projectTexCoord.x =  input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

	// ���� �� ��ǥ�� 0���� 1 ������ �ִ��� �����մϴ�. �׷��ٸ��� �ȼ��� ���� ������ �ֽ��ϴ�.
	if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		// ���� �� �ؽ�ó ��ǥ ��ġ���� ���÷��� ����Ͽ� ���� �ؽ�ó���� ������ �� ���� ���� ���ø��մϴ�.
		float depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

		// ���� ���̸� ����մϴ�.
		float lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

		// lightDepthValue���� ���̾�� ���ϴ�.
		lightDepthValue = lightDepthValue - bias;

		// ������ �� ���� ���̿� ���� ���̸� ���Ͽ��� �ȼ��� ���� ó������ �������� �����մϴ�.
		// ���� ��ü �տ� ������ �ȼ��� ���߰�, �׷��� ������ ��ü (��Ŭ ���)�� �׸��ڸ� �帮 ��� �������� �ȼ��� �׸��ڷ� �׸��ϴ�.
		if(lightDepthValue < depthValue)
		{
		    // �� �ȼ��� ���� ���� ����մϴ�.
			lightIntensity = saturate(dot(input.normal, lightDir));

		    if(lightIntensity > 0.0f)
			{
			    color = float4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}
	else
	{
		// �̰��� �׸��� �� ������ ���� �ۿ� �ִٸ� ���� �������� ������ ���������� �׸��ϴ�.
		lightIntensity = saturate(dot(input.normal, lightDir));
		if (lightIntensity > 0.0f)
		{
			color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

    return color;
}