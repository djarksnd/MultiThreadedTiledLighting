struct GS_Input
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

struct GS_Output
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
	// Shadow depth are stored texture array.
	// So renderTargetIndex is used as the texture array index.
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
	// Point light shadows require 6 face shadow depth textures.
	for (uint faceIndex = 0; faceIndex < 6; ++faceIndex)
	{
		GS_Output output[3];
		
		bool isItFront = false;
	
		for (uint vertex = 0; vertex < 3; ++vertex)
		{
			output[vertex].renderTargetIndex = renderTargetIndexOffset + faceIndex;
			output[vertex].position = mul(input[vertex].position, shadowMatrices[faceIndex]);
			output[vertex].texCoord = input[vertex].texCoord;

			if (output[vertex].position.w > 0.0f)
				isItFront = true;
		}

		if (isItFront)
		{
			for (uint vertex = 0; vertex < 3; ++vertex)
				outStream.Append(output[vertex]);
			
			outStream.RestartStrip();
		}
	}
}
