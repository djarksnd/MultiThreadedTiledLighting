#pragma once

#include "ConstantBuffer.h"
#include "DepthStencilBuffer.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "VertexShader.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "SamplerState.h"

class TiledRenderer;

class ShadowDepthBuffer
{
public:
	constexpr static unsigned int NumMaxPointLightShadows = 2;
	constexpr static unsigned int NumMaxSpotLightShadows = 8;

public:
	ShadowDepthBuffer();

	bool Create(const TiledRenderer& renderer, unsigned int bufferResolution);
	void Render(const TiledRenderer& renderer);
	void SetNumPointLightShadows(unsigned int numLights);
	void SetNumSpotLightShadows(unsigned int numLights);

	const DepthStencilBuffer& GetPointLightShadowDepthBuffer() const {
		return pointLightShadowDepthBuffer;
	}
	const DepthStencilBuffer& GetSpotLightShadowDepthBuffer() const {
		return spotLightShadowDepthBuffer;
	}
	unsigned int GetNumPointLightShadows() const {
		return numPointLightShadows;
	}
	unsigned int GetNumSpotLightShadows() const {
		return numSpotLightShadows;
	}

private:
	void RenderPointLightShadowDepth(const TiledRenderer& renderer);
	void RenderSpotLightShadowDepth(const TiledRenderer& renderer);

private:
	DepthStencilBuffer pointLightShadowDepthBuffer;
	DepthStencilBuffer spotLightShadowDepthBuffer;
	ConstantBuffer vertexShaderPerObjectCBuffer;
	ConstantBuffer vertexShaderPerLightCBuffer;
	ConstantBuffer geometryShaderCBuffer;
	VertexShader pointLightVertexShader;
	VertexShader spotLightVertexShader;
	GeometryShader geometryShader;
	PixelShader pixelShader;

	DepthStencilState depthStencilState;
	RasterizerState rasterizerState;
	SamplerState samplerState;

	unsigned int bufferResolution;
	unsigned int numPointLightShadows;
	unsigned int numSpotLightShadows;
};
