#include "ShadowDepthBuffer.h"
#include "TiledRenderer.h"
#include "MathHelper.h"
#include "Frustum.h"
#include "Object.h"
#include "Light.h"

struct GeometryShaderCBuffer
{
	DirectX::XMMATRIX shadowMatrices[6];
	uint32_t renderTargetIndexOffset;
	uint32_t padding[3];
};

struct VertexShaderPerLightCBuffer
{
	DirectX::XMMATRIX shadowMatrix;
	uint32_t renderTargetIndexOffset;
	uint32_t padding[3];
};

bool ShadowDepthBuffer::Create(const TiledRenderer& renderer,
							   unsigned int argBufferResolution)
{
	if (!pointLightShadowDepthBuffer.Create(renderer.GetDevice(), DepthStencilBuffer::Format::Depth32,
		argBufferResolution, argBufferResolution,
		NumMaxPointLightShadows * 6, 1, Texture::Dimension::Dimension2D))
		return false;

	if (!spotLightShadowDepthBuffer.Create(renderer.GetDevice(), DepthStencilBuffer::Format::Depth32,
		argBufferResolution, argBufferResolution,
		NumMaxSpotLightShadows, 1, Texture::Dimension::Dimension2D))
		return false;

	if (!vertexShaderPerObjectCBuffer.Create(renderer.GetDevice(), sizeof(DirectX::XMMATRIX)))
		return false;

	if (!vertexShaderPerLightCBuffer.Create(renderer.GetDevice(), sizeof(VertexShaderPerLightCBuffer)))
		return false;

	if (!geometryShaderCBuffer.Create(renderer.GetDevice(), sizeof(GeometryShaderCBuffer)))
		return false;

	const D3D11_INPUT_ELEMENT_DESC inputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	{
		std::vector<Shader::Macro> macro;
		macro.push_back({ "SpotLight", "0" });
		if (!pointLightVertexShader.Create(renderer.GetDevice(),
			L"..\\media\\shader\\ShadowDepthVertexShader.hlsl", "main", inputElements, ARRAYSIZE(inputElements), macro))
			return false;
	}

	{
		std::vector<Shader::Macro> macro;
		macro.push_back({ "SpotLight", "1" });
		if (!spotLightVertexShader.Create(renderer.GetDevice(),
			L"..\\media\\shader\\ShadowDepthVertexShader.hlsl", "main", inputElements, ARRAYSIZE(inputElements), macro))
			return false;
	}

	if (!geometryShader.Create(renderer.GetDevice(),
		L"..\\media\\shader\\ShadowDepthGeometryShader.hlsl", "main"))
		return false;

	if (!pixelShader.Create(renderer.GetDevice(),
		L"..\\media\\shader\\ShadowDepthPixelShader.hlsl", "main"))
		return false;

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0;
	dsDesc.StencilWriteMask = 0;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	if (!depthStencilState.Create(renderer.GetDevice(), dsDesc))
		return false;

	D3D11_RASTERIZER_DESC rsDesc;
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.FrontCounterClockwise = true;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.SlopeScaledDepthBias = -1.0f;
	rsDesc.DepthClipEnable = true;
	rsDesc.ScissorEnable = false;
	rsDesc.MultisampleEnable = false;
	rsDesc.AntialiasedLineEnable = false;
	if (!rasterizerState.Create(renderer.GetDevice(), rsDesc))
		return false;

	D3D11_SAMPLER_DESC ssDesc;
	ssDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ssDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	ssDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	ssDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ssDesc.MipLODBias = 0;
	ssDesc.MaxAnisotropy = 1;
	ssDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	ssDesc.BorderColor[0] = 0.0f;
	ssDesc.BorderColor[1] = 0.0f;
	ssDesc.BorderColor[2] = 0.0f;
	ssDesc.BorderColor[3] = 0.0f;
	ssDesc.MinLOD = -FLT_MAX;
	ssDesc.MaxLOD = FLT_MAX;
	if (!samplerState.Create(renderer.GetDevice(), ssDesc))
		return false;

	bufferResolution = argBufferResolution;

	return true;
}

void ShadowDepthBuffer::SetNumPointLightShadowLimit(size_t num)
{
	numPointLightShadowLimit = MathHelper::Clamp<size_t>(num, 0, NumMaxPointLightShadows);
}

void ShadowDepthBuffer::SetNumSpotLightShadowLimit(size_t num)
{
	numSpotLightShadowLimit = MathHelper::Clamp<size_t>(num, 0, NumMaxSpotLightShadows);
}

void ShadowDepthBuffer::RenderPointLightShadowDepth(const TiledRenderer& renderer)
{
	const Frustum frustum(renderer.GetViewInfo().invViewProjectionMatrix);
    size_t numShadows = MathHelper::Min(numPointLightShadowLimit, renderer.GetNumPointLightLimit());
    size_t renderTargetIndex = 0;
	for (size_t index = 0; index < numShadows; ++index)
	{
		const PointLight& light = renderer.GetPointLight(index);
		if (!frustum.Test(DirectX::XMLoadFloat3(&light.position), light.radius))
			continue;

		renderer.PostRenderTask([this, &renderer, &light, renderTargetIndex]() {
			ID3D11DeviceContext* deviceContext = renderer.GetDeviceContext();

			deviceContext->IASetInputLayout(pointLightVertexShader.GetInputLayout());
			deviceContext->VSSetShader(pointLightVertexShader, nullptr, 0);
			deviceContext->GSSetShader(geometryShader, nullptr, 0);
			deviceContext->OMSetRenderTargets(0, nullptr, pointLightShadowDepthBuffer);

			deviceContext->OMSetDepthStencilState(depthStencilState, 0);
			deviceContext->RSSetState(rasterizerState);

			D3D11_VIEWPORT viewPort;
			viewPort.TopLeftX = 0;
			viewPort.TopLeftY = 0;
			viewPort.Width = static_cast<float>(bufferResolution);
			viewPort.Height = static_cast<float>(bufferResolution);
			viewPort.MinDepth = 0.0f;
			viewPort.MaxDepth = 1.0f;
			deviceContext->RSSetViewports(1, &viewPort);

			ID3D11SamplerState* sampler = samplerState;
			deviceContext->PSSetSamplers(0, 1, &sampler);

			GeometryShaderCBuffer gsCbufferData;
			gsCbufferData.renderTargetIndexOffset = static_cast<uint32_t>(renderTargetIndex * 6);

			for (size_t face = 0; face < 6; ++face)
			{
				gsCbufferData.shadowMatrices[face] = ComputePointLightShadowMatrix(light, static_cast<PointLight::ShadowFace>(face));
				gsCbufferData.shadowMatrices[face] = DirectX::XMMatrixTranspose(gsCbufferData.shadowMatrices[face]);
			}

			geometryShaderCBuffer.Update(deviceContext, &gsCbufferData);
			ID3D11Buffer* buffer = geometryShaderCBuffer;
			deviceContext->GSSetConstantBuffers(0, 1, &buffer);

			deviceContext->PSSetShader(nullptr, nullptr, 0);

			const Frustum shadowFrustum = ComputePointLightShadowFrustum(light);

			for (auto& object : renderer.GetOpaqueObjects())
			{
				const DirectX::XMMATRIX matrix = DirectX::XMMatrixTranspose(object->GetTransform());
				vertexShaderPerObjectCBuffer.Update(deviceContext, &matrix);
				buffer = vertexShaderPerObjectCBuffer;
				deviceContext->VSSetConstantBuffers(0, 1, &buffer);

				object->Render(deviceContext, shadowFrustum);
			}

			deviceContext->PSSetShader(pixelShader, nullptr, 0);

			for (auto& object : renderer.GetMaskedObjects())
			{
				const DirectX::XMMATRIX matrix = DirectX::XMMatrixTranspose(object->GetTransform());
				vertexShaderPerObjectCBuffer.Update(deviceContext, &matrix);
				buffer = vertexShaderPerObjectCBuffer;
				deviceContext->VSSetConstantBuffers(0, 1, &buffer);

				object->Render(deviceContext, shadowFrustum);
			}
		});

		++renderTargetIndex;
	}

    numCurrFramePointLightShadows = renderTargetIndex;

	if (!renderer.GetEnableMultiThreadedRendering())
	{
		ID3D11DeviceContext* deviceContext = renderer.GetDeviceContext();
		deviceContext->VSSetShader(nullptr, nullptr, 0);
		deviceContext->GSSetShader(nullptr, nullptr, 0);
		deviceContext->PSSetShader(nullptr, nullptr, 0);
		deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		deviceContext->OMSetDepthStencilState(nullptr, 0);
		deviceContext->RSSetState(nullptr);
		ID3D11SamplerState* sampler = nullptr;
		deviceContext->PSSetSamplers(0, 1, &sampler);
		ID3D11Buffer* buffer = nullptr;
		deviceContext->VSSetConstantBuffers(0, 1, &buffer);
		deviceContext->GSSetConstantBuffers(0, 1, &buffer);
	}
}

void ShadowDepthBuffer::RenderSpotLightShadowDepth(const TiledRenderer& renderer)
{
    const size_t numShadows = MathHelper::Min(numSpotLightShadowLimit, renderer.GetNumSpotLightLimit());
	const Frustum frustum(renderer.GetViewInfo().invViewProjectionMatrix);
    size_t renderTargetIndex = 0;
	for (size_t index = 0; index < numShadows; ++index)
	{
		const SpotLight& light = renderer.GetSpotLight(index);
		if (!frustum.Test(DirectX::XMLoadFloat3(&light.position), light.radius))
			continue;

		renderer.PostRenderTask([this, &renderer, &light, renderTargetIndex]() {
			ID3D11DeviceContext* deviceContext = renderer.GetDeviceContext();

			deviceContext->IASetInputLayout(spotLightVertexShader.GetInputLayout());
			deviceContext->VSSetShader(spotLightVertexShader, nullptr, 0);
			deviceContext->OMSetRenderTargets(0, nullptr, spotLightShadowDepthBuffer);

			deviceContext->OMSetDepthStencilState(depthStencilState, 0);
			deviceContext->RSSetState(rasterizerState);

			D3D11_VIEWPORT viewPort;
			viewPort.TopLeftX = 0;
			viewPort.TopLeftY = 0;
			viewPort.Width = static_cast<float>(bufferResolution);
			viewPort.Height = static_cast<float>(bufferResolution);
			viewPort.MinDepth = 0.0f;
			viewPort.MaxDepth = 1.0f;
			deviceContext->RSSetViewports(1, &viewPort);

			ID3D11SamplerState* sampler = samplerState;
			deviceContext->PSSetSamplers(0, 1, &sampler);

			deviceContext->PSSetShader(nullptr, nullptr, 0);

			ID3D11Buffer* cbuffers[] = { vertexShaderPerObjectCBuffer, vertexShaderPerLightCBuffer };
			deviceContext->VSSetConstantBuffers(0, ARRAYSIZE(cbuffers), cbuffers);

			const DirectX::XMMATRIX shadowMatrix = ComputeSpotLightShadowMatrix(light);

			VertexShaderPerLightCBuffer cbufferData;
			cbufferData.shadowMatrix = DirectX::XMMatrixTranspose(shadowMatrix);
			cbufferData.renderTargetIndexOffset = static_cast<uint32_t>(renderTargetIndex);
			vertexShaderPerLightCBuffer.Update(deviceContext, &cbufferData);

			const Frustum shadowFrustum(DirectX::XMMatrixInverse(nullptr, shadowMatrix));

			for (auto& object : renderer.GetOpaqueObjects())
			{
				const DirectX::XMMATRIX matrix = DirectX::XMMatrixTranspose(object->GetTransform());
				vertexShaderPerObjectCBuffer.Update(deviceContext, &matrix);
				object->Render(deviceContext, shadowFrustum);
			}

			deviceContext->PSSetShader(pixelShader, nullptr, 0);

			for (auto& object : renderer.GetMaskedObjects())
			{
				const DirectX::XMMATRIX matrix = DirectX::XMMatrixTranspose(object->GetTransform());
				vertexShaderPerObjectCBuffer.Update(deviceContext, &matrix);
				object->Render(deviceContext, shadowFrustum);
			}
		});

		++renderTargetIndex;
	}

    numCurrFrameSpotLightShadows = renderTargetIndex;

	if (!renderer.GetEnableMultiThreadedRendering())
	{
		ID3D11DeviceContext* deviceContext = renderer.GetDeviceContext();
		deviceContext->VSSetShader(nullptr, nullptr, 0);
		deviceContext->PSSetShader(nullptr, nullptr, 0);
		deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		deviceContext->OMSetDepthStencilState(nullptr, 0);
		deviceContext->RSSetState(nullptr);
		ID3D11SamplerState* sampler = nullptr;
		deviceContext->PSSetSamplers(0, 1, &sampler);
		ID3D11Buffer* buffers[] = { nullptr, nullptr };
		deviceContext->VSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
	}
}

void ShadowDepthBuffer::Render(const TiledRenderer& renderer)
{
	ID3D11DeviceContext* deviceContext = renderer.GetDeviceContext();
	pointLightShadowDepthBuffer.Clear(deviceContext, 0.0f, 0);
	spotLightShadowDepthBuffer.Clear(deviceContext, 0.0f, 0);

	RenderPointLightShadowDepth(renderer);
	RenderSpotLightShadowDepth(renderer);
}
