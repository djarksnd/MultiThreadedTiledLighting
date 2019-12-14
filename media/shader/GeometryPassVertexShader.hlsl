struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float3 tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float3 tangent : TEXCOORD1;
};

cbuffer cbPerFrame : register(b0)
{
	float4x4 viewProjectionMatrix;
};

cbuffer cbPerDraw : register(b1)
{
	float4x4 worldMatrix;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.position = mul(mul(float4(input.position, 1.0f), worldMatrix), viewProjectionMatrix);
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.texCoord = input.texCoord;

	return output;
}
