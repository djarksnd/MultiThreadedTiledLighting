struct VS_Input
{
	float2 position : POSITION;
};

struct VS_Output
{
	float4 position : SV_Position;
	float2 positionCS : TEXCOORD0;
};

VS_Output main(VS_Input input)
{
	VS_Output output;

	output.position = float4(input.position, 0.0f, 1.0f);
	output.positionCS = input.position.xy;

	return output;
}