////////////////////////////////////////////////////////////////////////////////
// Filename: shadow_vs.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};


//////////////////////
// CONSTANT BUFFERS //
//////////////////////
cbuffer LightBuffer2
{
    float3 lightPosition;
	float padding;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 lightViewPosition : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ShadowVertexShader(VertexInputType input)
{
    PixelInputType output;
        
	// 적절한 행렬 계산을 위해 위치 벡터를 4 단위로 변경합니다.
    input.position.w = 1.0f;

	// 월드, 뷰 및 투영 행렬에 대한 정점의 위치를 ​​계산합니다.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// 광원에 의해 보았을 때 vertice의 위치를 ​​계산합니다.
    output.lightViewPosition = mul(input.position, worldMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);

	// 픽셀 쉐이더의 텍스처 좌표를 저장한다.
    output.tex = input.tex;
    
	// 월드 행렬에 대해서만 법선 벡터를 계산합니다.
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    // 법선 벡터를 정규화합니다.
    output.normal = normalize(output.normal);

    // 세계의 정점 위치를 계산합니다.
    float4 worldPosition = mul(input.position, worldMatrix);

    // 빛의 위치와 세계의 정점 위치를 기반으로 빛의 위치를 ​​결정합니다.
    output.lightPos = lightPosition.xyz - worldPosition.xyz;

    // 라이트 위치 벡터를 정규화합니다.
    output.lightPos = normalize(output.lightPos);

	return output;
}