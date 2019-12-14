Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState texSampler : register(s0);

struct PS_INPUT
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
	float3 tangent : TEXCOORD1;
};

struct PS_OUTPUT
{
	float4 sceneColor : SV_Target0;
	float4 diffuseSpecular : SV_Target1;
	float4 normalGlossiness : SV_Target2;
};

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT Output;

	float4 diffuse = diffuseTexture.Sample(texSampler, input.texcoord);

#if AlphaTestEnable
	clip(diffuse.a - 0.333f);
#endif

	float3 normal = normalTexture.Sample(texSampler, input.texcoord).xyz * 2.0f - 1.0f;

	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	float3 biNormal = normalize(cross(input.normal, input.tangent));
	float3x3 btnMatrix = float3x3(biNormal, input.tangent, input.normal);
	normal = normalize(mul(normal, btnMatrix));

	Output.sceneColor = diffuse * 0.01f;
	Output.diffuseSpecular = float4(diffuse.rgb, 0.02f);
	Output.normalGlossiness = float4(normal * 0.5f + 0.5f, 60.0f);

	return Output;
}

