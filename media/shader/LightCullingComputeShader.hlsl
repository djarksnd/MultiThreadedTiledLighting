#include "Common.hlsl"

cbuffer PerDrawCBuffer : register(b0)
{
	matrix invProjectionMatrix;
	matrix viewMatrix;
	float viewWidth;
	float viewHeight;
	uint numPointLights;
	uint numSpotLights;
	uint numTilesX;
	uint numTilesY;
	uint2 padding;
};

Texture2D<float> depthBuffer : register(t0);
StructuredBuffer<PointLight> pointLightBuffer : register(t1);
StructuredBuffer<SpotLight> spotLightBuffer : register(t2);

RWBuffer<uint> lightIndexBuffer : register(u0);

// per-tile pointLight indices
groupshared uint pointLightCounter[2];
groupshared uint pointLightIndices[2][NumMaxPointLightsPerTile];

// per-tile spotLight indices
groupshared uint spotLightCounter[2];
groupshared uint spotLightIndices[2][NumMaxSpotLightsPerTile];

// view space depth min and max
groupshared uint minDepth;
groupshared uint maxDepth;

void CalcMinMaxAndHalfDepth(in uint3 dispatchThreadID,
							out float minViewDepth,
							out float maxViewDepth,
							out float halfViewDepth)
{
	const float sceneDepth = depthBuffer.Load(dispatchThreadID);
	if (sceneDepth != 0.0f)
	{
		const float viewDepth = CalcViewDepth(sceneDepth, invProjectionMatrix);
		const uint depth = asuint(viewDepth);
		InterlockedMin(minDepth, depth);
		InterlockedMax(maxDepth, depth);
	}

	GroupMemoryBarrierWithGroupSync();

	minViewDepth = asfloat(minDepth);
	maxViewDepth = asfloat(maxDepth);
	halfViewDepth = (minViewDepth + maxViewDepth) * 0.5f;
}

void CalcFrustum(in uint3 groupID, out float3 frustum[4])
{
	const float left = ((float)groupID.x / (float)numTilesX) * 2.0f - 1.0f;
	const float right = ((float)(groupID.x + 1) / (float)numTilesX) * 2.0f - 1.0f;
	const float top = ((float)groupID.y / (float)numTilesY) * -2.0f + 1.0f;
	const float bottom = ((float)(groupID.y + 1) / (float)numTilesY) * -2.0f + 1.0f;

	float4 vertices[4] = {
		float4(left, top, 1.0f, 1.0f),
		float4(right, top, 1.0f, 1.0f),
		float4(right, bottom, 1.0f, 1.0f),
		float4(left, bottom, 1.0f, 1.0f) };

	for (uint v = 0; v < 4; ++v)
	{
		vertices[v] = mul(vertices[v], invProjectionMatrix);
		vertices[v] /= vertices[v].w;
	}

	for (uint i = 0; i < 4; ++i)
	{
		frustum[i] = normalize(cross(vertices[(i + 1) % 4].xyz, vertices[i].xyz));
	}
}

void CullLights(in uint3 dispatchThreadID,
				in uint3 groupID,
				in uint groupIndex,
				out float halfViewDepth)
{
	float3 frustum[4];
	CalcFrustum(groupID, frustum);

	float minViewDepth, maxViewDepth;
	CalcMinMaxAndHalfDepth(dispatchThreadID, minViewDepth, maxViewDepth, halfViewDepth);

	for (uint pointLightIndex = groupIndex; pointLightIndex < numPointLights; pointLightIndex += NumThreadsPerTile)
	{
		PointLight light = pointLightBuffer[pointLightIndex];
		light.position.xyz = mul(float4(light.position.xyz, 1.0f), viewMatrix).xyz;

		if (dot(light.position.xyz, frustum[0]) > -light.radius &&
			dot(light.position.xyz, frustum[1]) > -light.radius &&
			dot(light.position.xyz, frustum[2]) > -light.radius &&
			dot(light.position.xyz, frustum[3]) > -light.radius)
		{
			if (minViewDepth < (light.position.z + light.radius) &&
				halfViewDepth >(light.position.z - light.radius))
			{
				uint bufferIndex = 0;
				InterlockedAdd(pointLightCounter[0], 1, bufferIndex);
				pointLightIndices[0][bufferIndex] = pointLightIndex;
			}

			if (halfViewDepth < (light.position.z + light.radius) &&
				maxViewDepth > (light.position.z - light.radius))
			{
				uint bufferIndex = 0;
				InterlockedAdd(pointLightCounter[1], 1, bufferIndex);
				pointLightIndices[1][bufferIndex] = pointLightIndex;
			}
		}
	}

	for (uint spotLightIndex = groupIndex; spotLightIndex < numSpotLights; spotLightIndex += NumThreadsPerTile)
	{
		SpotLight light = spotLightBuffer[spotLightIndex];
		light.position.xyz = mul(float4(light.position.xyz, 1.0f), viewMatrix).xyz;

		if (dot(light.position.xyz, frustum[0]) > -light.radius &&
			dot(light.position.xyz, frustum[1]) > -light.radius &&
			dot(light.position.xyz, frustum[2]) > -light.radius &&
			dot(light.position.xyz, frustum[3]) > -light.radius)
		{
			if (minViewDepth < (light.position.z + light.radius) &&
				halfViewDepth >(light.position.z - light.radius))
			{
				uint bufferIndex = 0;
				InterlockedAdd(spotLightCounter[0], 1, bufferIndex);
				spotLightIndices[0][bufferIndex] = spotLightIndex;
			}

			if (halfViewDepth < (light.position.z + light.radius) &&
				maxViewDepth > (light.position.z - light.radius))
			{
				uint bufferIndex = 0;
				InterlockedAdd(spotLightCounter[1], 1, bufferIndex);
				spotLightIndices[1][bufferIndex] = spotLightIndex;
			}
		}
	}
}

[numthreads(NumThreadsPerTileX, NumThreadsPerTileY, 1)]
void main(in uint3 dispatchThreadID : SV_DispatchThreadID,
		  in uint3 groupID : SV_GroupID,
		  in uint groupIndex : SV_GroupIndex)
{
	if (groupIndex == 0)
	{
		minDepth = 0x7f7fffff;  // FLT_MAX as a uint;
		maxDepth = 0;
		pointLightCounter[0] = 0;
		pointLightCounter[1] = 0;
		spotLightCounter[0] = 0;
		spotLightCounter[1] = 0;
	}

	GroupMemoryBarrierWithGroupSync();

	float halfViewDepth;
	CullLights(dispatchThreadID, groupID, groupIndex, halfViewDepth);

	GroupMemoryBarrierWithGroupSync();

	uint offset = ((groupID.y * numTilesX) + groupID.x) * 
		(((NumMaxPointLightsPerTile + NumMaxSpotLightsPerTile) * 2) + 6);

	if (groupIndex == 0)
	{
		const uint halfDepthBits = asuint(halfViewDepth);

		lightIndexBuffer[offset] = halfDepthBits >> 16;
		lightIndexBuffer[offset + 1] = halfDepthBits & 0xffff;
		lightIndexBuffer[offset + 2] = pointLightCounter[0];
		lightIndexBuffer[offset + 3] = pointLightCounter[1];
		lightIndexBuffer[offset + 4] = spotLightCounter[0];
		lightIndexBuffer[offset + 5] = spotLightCounter[1];
	}

	offset += 6;

	for (uint counter = 0; counter < 2; ++counter)
	{
		for (uint index = groupIndex; index < pointLightCounter[counter]; index += NumThreadsPerTile)
		{
			lightIndexBuffer[offset + index] = pointLightIndices[counter][index];
		}

		offset += NumMaxPointLightsPerTile;
	}

	for (uint counter = 0; counter < 2; ++counter)
	{
		for (uint index = groupIndex; index < spotLightCounter[counter]; index += NumThreadsPerTile)
		{
			lightIndexBuffer[offset + index] = spotLightIndices[counter][index];
		}

		offset += NumMaxSpotLightsPerTile;
	}
}