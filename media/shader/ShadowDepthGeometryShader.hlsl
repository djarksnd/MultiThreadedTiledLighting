struct GS_Input
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

struct GS_Output
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
	uint renderTargetIndex : SV_RenderTargetArrayIndex;
};

cbuffer cbPerDraw : register(b0)
{
	float4x4 shadowMatrices[6];
	uint renderTargetIndexOffset;
	uint padding[3];
};

[maxvertexcount(18)]
void main(triangle GS_Input input[3], inout TriangleStream<GS_Output> outStream)
{
	for (uint face = 0; face < 6; ++face)
	{
		GS_Output output[3];
		
		bool test = false;
	
		for (uint vertex = 0; vertex < 3; ++vertex)
		{
			output[vertex].renderTargetIndex = face + renderTargetIndexOffset;
			output[vertex].position = mul(input[vertex].position, shadowMatrices[face]);
			output[vertex].texCoord = input[vertex].texCoord;

			if (output[vertex].position.w > 0.0f)
				test = true;
		}

		if (test)
		{
			for (uint vertex = 0; vertex < 3; ++vertex)
				outStream.Append(output[vertex]);
			
			outStream.RestartStrip();
		}
	}
}
