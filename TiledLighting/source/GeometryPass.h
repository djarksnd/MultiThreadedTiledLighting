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
		enum Type
		{
			DiffuseSpecular = 0,
			NormalGlossiness,

			NumTypes,
		};
	};

public:
	GeometryPass() {}

	bool Create(const TiledRenderer& renderer);
	bool Resize(const TiledRenderer& renderer);
	void Render(const TiledRenderer& renderer);

	const RenderTarget& GetBuffer(BufferType::Type type) const{
		return renderTargets[type];
	}

	GeometryPass(const GeometryPass&) = delete;
	const GeometryPass& operator = (const GeometryPass&) = delete;

private:
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