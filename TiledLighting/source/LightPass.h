#pragma once

#include <D3D11.h>
#include <vector>

#include "Buffer.h"
#include "BlendState.h"
#include "ComputeShader.h"
#include "ConstantBuffer.h"
#include "DepthStencilState.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "Light.h"

class TiledRenderer;

class LightPass
{
public:
	bool Create();
	bool Resize();
	void Render();

	LightPass(const TiledRenderer& renderer);
	LightPass(const LightPass&) = delete;
	const LightPass& operator = (const LightPass&) = delete;

private:
    // Cull lights with compute shader and screen align tile's frustums.
	void CullLights();
    // Calculate lighting with gbuffers and gpu lights culling results to scene color buffer.
	void RenderLights();

	static constexpr unsigned int NumMaxPointLightsPerTile = 128;
	static constexpr unsigned int NumMaxSpotLightsPerTile = 128;
	static constexpr unsigned int NumThreadsPerTileX = 32;
	static constexpr unsigned int NumThreadsPerTileY = 16;

	static constexpr unsigned int GetNumTileX(unsigned int screenWidth) {
		return (screenWidth + (NumThreadsPerTileX - 1)) / NumThreadsPerTileX;
	}
	static constexpr unsigned int GetNumTileY(unsigned int screenHeight) {
		return (screenHeight + (NumThreadsPerTileY - 1)) / NumThreadsPerTileY;
	}

private:
    const TiledRenderer& renderer;
	ComputeShader lightCullingComputeShader;
	ConstantBuffer lightCullingCSCBuffer;
	Buffer pointLightBuffer;
	Buffer spotLightBuffer;
	Buffer lightIndexBuffer;
	PixelShader deferredLightingPixelShader;
	PixelShader visualizeNumLightsPixelShader;
	ConstantBuffer deferredLightingPSCBuffer;
	DepthStencilState deferredLightingDepthStencilState;
	BlendState deferredLightingBlendState;
	SamplerState shadowDepthSamplerState;
	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
};
