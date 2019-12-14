Texture2D diffuseTexture : register(t0);
SamplerState texSampler : register(s0);

struct PS_INPUT
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

void main(PS_INPUT input)
{
	const float alpha = diffuseTexture.Sample(texSampler, input.texCoord).a;
	clip(alpha - 0.333f);
}

