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
	bool Create(const TiledRenderer& renderer);
	bool Resize(const TiledRenderer& renderer);
	void Render(const TiledRenderer& renderer);

	LightPass() {}
	LightPass(const LightPass&) = delete;
	const LightPass& operator = (const LightPass&) = delete;

private:
	void CullLights(const TiledRenderer& renderer);
	void RenderLights(const TiledRenderer& renderer);

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