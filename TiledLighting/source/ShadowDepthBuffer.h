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
	bool Create(unsigned int bufferResolution);
    bool Resize(unsigned int bufferResolution);
	void Render();
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

    ShadowDepthBuffer(const TiledRenderer& renderer);
    ShadowDepthBuffer(const ShadowDepthBuffer&) = delete;
    const ShadowDepthBuffer& operator = (const ShadowDepthBuffer&) = delete;

private:
	void RenderPointLightShadowDepth();
	void RenderSpotLightShadowDepth();

private:
    const TiledRenderer& renderer;
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

    float bufferResolution = 0.0f;
    size_t numPointLightShadowLimit = NumMaxPointLightShadows;
    size_t numSpotLightShadowLimit = NumMaxSpotLightShadows;
    size_t numCurrFramePointLightShadows = 0;
    size_t numCurrFrameSpotLightShadows = 0;
};
