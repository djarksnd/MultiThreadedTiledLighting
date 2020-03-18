#include "GeometryPass.h"
#include "Frustum.h"
#include "TiledRenderer.h"
#include "Object.h"
#include "DepthStencilBuffer.h"



bool GeometryPass::Create(const TiledRenderer& renderer)
{
	if (!Resize(renderer))
		return false;

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0x00;
	dsDesc.StencilWriteMask = TiledRenderer::StenCilMask::DeferredLightable;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	if (!depthStencilState.Create(renderer.GetDevice(), dsDesc))
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

	const D3D11_INPUT_ELEMENT_DESC inputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (!vertexShader.Create(renderer.GetDevice(), L"..\\media\\shader\\GeometryPassVertexShader.hlsl", "main", inputElements, ARRAYSIZE(inputElements)))
		return false;

	std::vector<Shader::Macro> opaquePSMacro = { {"AlphaTestEnable", "0"} };
	if (!opaquePixelShader.Create(renderer.GetDevice(), L"..\\media\\shader\\GeometryPassPixelShader.hlsl", "main", opaquePSMacro))
		return false;

	std::vector<Shader::Macro> alphaTestPSmacro = { {"AlphaTestEnable", "1"} };
	if (!maskedPixelShader.Create(renderer.GetDevice(), L"..\\media\\shader\\GeometryPassPixelShader.hlsl", "main", alphaTestPSmacro))
		return false;

	if (!vsPerFrameCbuffer.Create(renderer.GetDevice(), sizeof(DirectX::XMMATRIX)))
		return false;

	if (!vsPerDrawCbuffer.Create(renderer.GetDevice(), sizeof(DirectX::XMMATRIX)))
		return false;

	return true;
}

bool GeometryPass::Resize(const TiledRenderer& renderer)
{
	renderTargets[BufferType::DiffuseSpecular].Destroy();
	if (!renderTargets[BufferType::DiffuseSpecular].Create(renderer.GetDevice(),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		ResourceBindFlags::RenderTargetBit | ResourceBindFlags::ShaderResourceBit,
		renderer.GetScreenWidth(),
		renderer.GetScreenHeight(),
		1,
		renderer.GetNumSubSamples()))
		return false;

	renderTargets[BufferType::NormalGlossiness].Destroy();
	if (!renderTargets[BufferType::NormalGlossiness].Create(renderer.GetDevice(),
		DXGI_FORMAT_R10G10B10A2_UNORM,
		ResourceBindFlags::RenderTargetBit | ResourceBindFlags::ShaderResourceBit,
		renderer.GetScreenWidth(),
		renderer.GetScreenHeight(),
		1,
		renderer.GetNumSubSamples()))
		return false;

	D3D11_RASTERIZER_DESC rsDesc;
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.FrontCounterClockwise = true;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	rsDesc.DepthClipEnable = true;
	rsDesc.ScissorEnable = false;
	rsDesc.MultisampleEnable = renderer.GetNumSubSamples() > 1;
	rsDesc.AntialiasedLineEnable = false;

	rasterizerState.Destroy();
	if (!rasterizerState.Create(renderer.GetDevice(), rsDesc))
		return false;

	return true;
}

void GeometryPass::Render(const TiledRenderer& renderer)
{
    // Drawing geometry buffer.
	renderer.PostRenderTask([this, &renderer]() {
		ID3D11DeviceContext* deviceContext = renderer.GetDeviceContext();

		D3D11_VIEWPORT viewPort;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		viewPort.Width = static_cast<float>(renderer.GetScreenWidth());
		viewPort.Height = static_cast<float>(renderer.GetScreenHeight());
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;
		deviceContext->RSSetViewports(1, &viewPort);

		ID3D11RenderTargetView* rtvs[] = {
			renderer.GetSceneRenderTarget(),
			renderTargets[BufferType::DiffuseSpecular],
			renderTargets[BufferType::NormalGlossiness] };

		deviceContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, renderer.GetSceneDepthStencilBuffer());
		deviceContext->OMSetDepthStencilState(depthStencilState, TiledRenderer::StenCilMask::DeferredLightable);
		deviceContext->RSSetState(rasterizerState);

		deviceContext->IASetInputLayout(vertexShader.GetInputLayout());
		deviceContext->VSSetShader(vertexShader, nullptr, 0);

		ID3D11SamplerState* sampler = samplerState;
		deviceContext->PSSetSamplers(0, 1, &sampler);

		DirectX::XMMATRIX viewProjectionMatrix = DirectX::XMMatrixTranspose(renderer.GetViewInfo().viewProjectionMatrix);
		vsPerFrameCbuffer.Update(deviceContext, &viewProjectionMatrix);
		ID3D11Buffer* buffer = vsPerFrameCbuffer;
		deviceContext->VSSetConstantBuffers(0, 1, &buffer);

		const Frustum frustum(renderer.GetViewInfo().invViewProjectionMatrix);

        // Drawing opaque objects geometries.
		deviceContext->PSSetShader(opaquePixelShader, nullptr, 0);
		for (auto& object : renderer.GetOpaqueObjects())
		{
			DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(object->GetTransform());
			vsPerDrawCbuffer.Update(deviceContext, &world);
			buffer = vsPerDrawCbuffer;
			deviceContext->VSSetConstantBuffers(1, 1, &buffer);

			object->Render(deviceContext, frustum);
		}

        // drawing masked objects geometries.
		deviceContext->PSSetShader(maskedPixelShader, nullptr, 0);
		for (auto& object : renderer.GetMaskedObjects())
		{
			DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(object->GetTransform());
			vsPerDrawCbuffer.Update(deviceContext, &world);
			buffer = vsPerDrawCbuffer;
			deviceContext->VSSetConstantBuffers(1, 1, &buffer);

			object->Render(deviceContext, frustum);
		}

        for (auto& rtv : rtvs)
            rtv = nullptr;

		deviceContext->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, nullptr);
		deviceContext->OMSetDepthStencilState(nullptr, 0);
		deviceContext->RSSetState(nullptr);
		deviceContext->IASetInputLayout(nullptr);

		deviceContext->VSSetShader(nullptr, nullptr, 0);
		deviceContext->PSSetShader(nullptr, nullptr, 0);

		sampler = nullptr;
		deviceContext->PSSetSamplers(0, 1, &sampler);

		ID3D11Buffer* buffers[2] = { nullptr };
		deviceContext->VSSetConstantBuffers(0, 2, buffers);
	});
}
