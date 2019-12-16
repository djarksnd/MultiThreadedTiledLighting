#include "Common.hlsl"

cbuffer PerDrawCBuffer : register(b0)
{
	matrix projectionMatrix;
	matrix invViewProjectionMatrix;
	float3 cameraPosition;
	uint numThreadsPerTileX;
	uint numThreadsPerTileY;
	uint numTilesX;
	uint numPointLightShadows;
	uint numSpotLightShadows;
	matrix pointLightShadowMatrices[NumMaxPointLightShadows][6];
	matrix spotLightShadowMatrices[NumMaxSpotLightShadows];
};

Texture2D<float> sceneDepthBuffer : register(t0);
Texture2D<float4> diffuseSpecularBuffer : register(t1);
Texture2D<float4> normalGlossinessBuffer : register(t2);
Buffer<uint> lightIndexBuffer : register(t3);
StructuredBuffer<PointLight> pointLightBuffer : register(t4);
StructuredBuffer<SpotLight> spotLightBuffer : register(t5);
Texture2DArray<float> pointLightShadowDepthBuffer : register(t6);
Texture2DArray<float> spotLightShadowDepthBuffer : register(t7);

SamplerComparisonState shadowDepthSampler : register(s0);

struct PS_Input
{
	float4 position : SV_Position;
	float2 positionCS : TEXCOORD0;
};

float CalcPointLightShadow(in float4 worldPosition, uint lightIndex)
{
	if (lightIndex < numPointLightShadows)
	{
		for (uint face = 0; face < 6; ++face)
		{
			float4 shadowPosition = mul(worldPosition, pointLightShadowMatrices[lightIndex][face]);
			shadowPosition /= shadowPosition.w;

			if (shadowPosition.x >= -1.0f && shadowPosition.x <= 1.0f &&
				shadowPosition.y >= -1.0f && shadowPosition.y <= 1.0f &&
				shadowPosition.z >= 0.0f && shadowPosition.z <= 1.0f)
			{
				const float3 texCoord = float3(shadowPosition.xy * float2(0.5f, -0.5f) + 0.5f, (lightIndex * 6) + face);

				float shadow = 0.0f;
				const int halfKernelSize = 2;
				const int KernelSize = (halfKernelSize * 2) + 1;

				for (int y = -halfKernelSize; y <= halfKernelSize; ++y)
				{
					for (int x = -halfKernelSize; x <= halfKernelSize; ++x)
					{
						shadow += pointLightShadowDepthBuffer.SampleCmpLevelZero(
							shadowDepthSampler,
							texCoord,
							shadowPosition.z + 0.000006f,
							int3(x, y, 0)).r;
					}
				}

				return shadow / float(KernelSize * KernelSize);
			}
		}
	}

	return 1.0f;
}

float CalcSpotLightShadow(in float4 worldPosition, uint lightIndex)
{
	if (lightIndex < numSpotLightShadows)
	{
		float4 shadowPosition = mul(worldPosition, spotLightShadowMatrices[lightIndex]);
		shadowPosition /= shadowPosition.w;

		if (shadowPosition.x >= -1.0f && shadowPosition.x <= 1.0f &&
			shadowPosition.y >= -1.0f && shadowPosition.y <= 1.0f &&
			shadowPosition.z >= 0.0f && shadowPosition.z <= 1.0f)
		{
			const float3 texCoord = float3(shadowPosition.xy * float2(0.5f, -0.5f) + 0.5f, lightIndex);

			float shadow = 0.0f;
			const int halfKernelSize = 2;
			const int KernelSize = (halfKernelSize * 2) + 1;

			for (int y = -halfKernelSize; y <= halfKernelSize; ++y)
			{
				for (int x = -halfKernelSize; x <= halfKernelSize; ++x)
				{
					shadow += spotLightShadowDepthBuffer.SampleCmpLevelZero(
						shadowDepthSampler,
						texCoord,
						shadowPosition.z + 0.000002f,
						int3(x, y, 0)).r;
				}
			}

			return shadow / float(KernelSize * KernelSize);
		}
	}

	return 1.0f;
}

[earlydepthstencil]
float3 main(in PS_Input input) : SV_Target
{
	float3 result = 0.0f;

	const int3 location = int3(input.position.xy, 0);

	float4 diffuseSpecular = diffuseSpecularBuffer.Load(location);
	float4 normalGlossiness = normalGlossinessBuffer.Load(location);
	normalGlossiness.xyz = normalGlossiness.xyz * 2.0f - 1.0f;

	const float sceneDepth = sceneDepthBuffer.Load(location);
	float4 worldPosition = mul(float4(input.positionCS, sceneDepth, 1.0f), invViewProjectionMatrix);
	worldPosition /= worldPosition.w;

	const float3 view = cameraPosition - worldPosition.xyz;
	
	const uint2 tileIndex = uint2(location.x / numThreadsPerTileX, location.y / numThreadsPerTileY);
	uint offset = ((tileIndex.y * numTilesX) + tileIndex.x) * (((NumMaxPointLightsPerTile + NumMaxSpotLightsPerTile) * 2) + 6);

	const float halfDepth = asfloat((lightIndexBuffer[offset++] << 16) | lightIndexBuffer[offset++]);
	const uint pointLightCounter[2] = {lightIndexBuffer[offset++], lightIndexBuffer[offset++]};
	const uint spotLightCounter[2] = {lightIndexBuffer[offset++], lightIndexBuffer[offset++]};
	const float viewDepth = projectionMatrix._43 / (sceneDepth - projectionMatrix._33);
	const uint counterIndex = (viewDepth < halfDepth) ? 0 : 1;

	uint begin = offset + (counterIndex * NumMaxPointLightsPerTile);
	uint end = begin + pointLightCounter[counterIndex];

#if VisualizeNumLights
	float numLights = float(end - begin) / float(NumMaxPointLightsPerTile);
#else
	for (uint index = begin; index < end; ++index)
	{
		const uint lightIndex = lightIndexBuffer[index];
		const float shadow = CalcPointLightShadow(worldPosition, lightIndex);
		result += CalcPointLighting(pointLightBuffer[lightIndex],
									worldPosition.xyz, 
									view, 
									normalGlossiness.xyz, 
									diffuseSpecular.rgb, 
									diffuseSpecular.a, 
									normalGlossiness.w) * shadow;
	}
#endif

	offset += (NumMaxPointLightsPerTile * 2);

	begin = offset + (counterIndex * NumMaxSpotLightsPerTile);
	end = begin + spotLightCounter[counterIndex];

#if VisualizeNumLights
	numLights += float(end - begin) / float(NumMaxSpotLightsPerTile);
	return saturate(numLights / 2.0f);
#else
	for (uint index = begin; index < end; ++index)
	{
		const uint lightIndex = lightIndexBuffer[index];
		const float shadow = CalcSpotLightShadow(worldPosition, lightIndex);
		result += CalcSpotLighting(spotLightBuffer[lightIndex],
								   worldPosition.xyz, 
								   view, 
								   normalGlossiness.xyz, 
								   diffuseSpecular.rgb,
								   diffuseSpecular.a,
								   normalGlossiness.w) * shadow;
	}
#endif

	return result;
}