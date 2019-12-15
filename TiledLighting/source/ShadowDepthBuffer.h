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
	constexpr static size_t NumMaxPointLightShadows = 2;
	constexpr static size_t NumMaxSpotLightShadows = 8;

public:
    ShadowDepthBuffer() {}

	bool Create(const TiledRenderer& renderer, unsigned int bufferResolution);
	void Render(const TiledRenderer& renderer);
	void SetNumPointLightShadowLimit(size_t num);
	void SetNumSpotLightShadowLimit(size_t num);

	const DepthStencilBuffer& GetPointLightShadowDepthBuffer() const {
		return pointLightShadowDepthBuffer;
	}
	const DepthStencilBuffer& GetSpotLightShadowDepthBuffer() const {
		return spotLightShadowDepthBuffer;
	}
    size_t GetNumPointLightShadowLimit() const {
        return numPointLightShadowLimit;
    }
    size_t GetNumSpotLightShadowLimit() const {
        return numSpotLightShadowLimit;
    }
    size_t GetNumCurrFramePointLightShadows() const {
		return numCurrFramePointLightShadows;
	}
    size_t GetNumCurrFrameSpotLightShadows() const {
		return numCurrFrameSpotLightShadows;
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

    unsigned int bufferResolution = 0;
    size_t numPointLightShadowLimit = NumMaxPointLightShadows;
    size_t numSpotLightShadowLimit = NumMaxSpotLightShadows;
    size_t numCurrFramePointLightShadows = 0;
    size_t numCurrFrameSpotLightShadows = 0;
};
