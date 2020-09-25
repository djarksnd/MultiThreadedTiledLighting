#pragma once

#include <D3D11.h>
#include <array>

#include "RenderTarget.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "ConstantBuffer.h"
#include "SamplerState.h"
#include "PixelShader.h"
#include "VertexShader.h"

class TiledRenderer;

class GeometryPass
{
public:
	struct BufferType
	{
        // DiffuseSpecular buffer format [R8G8B8A8_UNORM]
        // DiffuseSpecular.RGB = DiffuseColor.RGB
        // DiffuseSpecular.A = Specular intensity

        // NormalGlossiness buffer format [R10G10B10A2_UNORM]
        // NormalGlossiness.RG = Normal.XY;
        // NormalGlossiness.B = Specular power;
        // NormalGlossiness.A = Normal Z sign;
		enum Type
		{
			DiffuseSpecular = 0,
			NormalGlossiness,

			NumTypes,
		};
	};

public:
	bool Create();
	bool Resize();
	void Render();

	const RenderTarget& GetBuffer(BufferType::Type type) const{
		return renderTargets[type];
	}

    GeometryPass(const TiledRenderer& renderer);
	GeometryPass(const GeometryPass&) = delete;
	const GeometryPass& operator = (const GeometryPass&) = delete;

private:
    const TiledRenderer& renderer;
	std::array<RenderTarget, BufferType::NumTypes> renderTargets;
	RasterizerState rasterizerState;
	DepthStencilState depthStencilState;
	VertexShader vertexShader;
	ConstantBuffer vsPerFrameCbuffer;
	ConstantBuffer vsPerDrawCbuffer;
	PixelShader opaquePixelShader;
	PixelShader maskedPixelShader;
	SamplerState samplerState;
};
