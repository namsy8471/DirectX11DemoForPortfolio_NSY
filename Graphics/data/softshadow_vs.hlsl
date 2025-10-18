////////////////////////////////////////////////////////////////////////////////
// Filename: softshadow_vs.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
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
	float3 normal : NORMAL;          // view-space normal
    float4 viewPosition : TEXCOORD1; // clip-space position for screen-space sampling
	float3 lightPos : TEXCOORD2;     // view-space light direction
	float3 viewDir : TEXCOORD3;      // view-space view direction
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType SoftShadowVertexShader(VertexInputType input)
{
    PixelInputType output;
    
	// 적절한 행렬 계산을 위해 위치 벡터를 4 단위로 변경합니다.
    input.position.w = 1.0f;

	// World and view positions
    float4 worldPosition = mul(input.position, worldMatrix);
    float4 viewPos = mul(worldPosition, viewMatrix);

    // Output SV_POSITION (clip space) and also forward the same as TEXCOORD for sampling
    float4 clipPosition = mul(viewPos, projectionMatrix);
    output.position = clipPosition;
    output.viewPosition = clipPosition;
	
	// Transform normal to view space (approximate: world then view, assuming no non-uniform scale)
    float3 worldNormal = mul(input.normal, (float3x3)worldMatrix);
    float3 viewNormal = mul(worldNormal, (float3x3)viewMatrix);
	output.normal = normalize(viewNormal);

    // Compute light direction in view space
    float3 worldLightDir = lightPosition.xyz - worldPosition.xyz;
    float3 viewLightDir = mul(worldLightDir, (float3x3)viewMatrix);
    output.lightPos = normalize(viewLightDir);

    // View direction (camera at (0,0,0) in view space)
    output.viewDir = normalize(-viewPos.xyz);

	// Pass through texcoords
	output.tex = input.tex;

	return output;
}