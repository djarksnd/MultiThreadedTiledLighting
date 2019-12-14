cbuffer PostprocessCBuffer
{
	float invGamma;
	float3 padding;
};

Texture2D<float3> sceneColorBuffer;

float4 main(float4 position : SV_Position) : SV_Target
{
	const uint3 location = position;
	float3 sceneColor = sceneColorBuffer.Load(location);

	float gamma = 1.0f;
	sceneColor = pow(sceneColor, invGamma);

	return float4(saturate(sceneColor), 1.0f);
}