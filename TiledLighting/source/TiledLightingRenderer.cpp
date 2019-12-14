#include <random>
#include <DirectXMath.h>

#include "..\\..\\DXUT\\Core\\DXUT.h"
#include "..\\..\\DXUT\\Optional\\DXUTCamera.h"

#include "TiledLightingRenderer.h"
#include "ViewInfo.h"
#include "MathHelper.h"

bool TiledLightingRenderer::Create(ID3D11Device* device, unsigned int argScreenWidth,
								   unsigned int argScreenHeight, unsigned numSubSamples)
{
	InitObjects(device);
	InitLights();

	if (!geometryPass.Create(device, argScreenWidth, argScreenHeight, numSubSamples))
		return false;

	if (!lightPass.Create(device, argScreenWidth, argScreenHeight, numSubSamples, pointLights.size(), spotLights.size()))
		return false;

	if (!sceneRenderTarget.Create(
		device,
		DXGI_FORMAT_R11G11B10_FLOAT,
		GPUResourceBindFlags::RenderTargetBit | GPUResourceBindFlags::ShaderResourceBit,
		argScreenWidth,
		argScreenHeight,
		1,
		numSubSamples))
		return false;

	if (!sceneDepthStencilBuffer.Create(device,
		DepthStencilBuffer::Format::Depth24_Stencil8,
		argScreenWidth,
		argScreenHeight,
		numSubSamples))
		return false;

	screenWidth = argScreenWidth;
	screenHeight = argScreenHeight;

	return true;
}

void TiledLightingRenderer::InitObjects(ID3D11Device* device)
{

}

void TiledLightingRenderer::InitLights()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, 1024);

	const DirectX::XMFLOAT3 sceneMin(-3781.37256f, -904.380310f, 2249.54248f);
	const DirectX::XMFLOAT3 sceneMax(3660.33496f, 2207.37085f, 2326.92334f);
	
	pointLights.resize(1024);
	for (auto& light : pointLights)
	{
		light.position.x = Lerp(sceneMin.x, sceneMax.x, static_cast<float>(dist(gen)) / 1024.0f);
		light.position.y = Lerp(sceneMin.y, sceneMax.y, static_cast<float>(dist(gen)) / 1024.0f);
		light.position.z = Lerp(sceneMin.z, sceneMax.z, static_cast<float>(dist(gen)) / 1024.0f);
		
		light.radius = Lerp(10.0f, 300.0f, static_cast<float>(dist(gen)) / 1024.0f);
		
		light.color.r = Lerp(0.5f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);
		light.color.g = Lerp(0.5f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);
		light.color.b = Lerp(0.5f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);
		
		light.falloff = Lerp(1.0f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);
	}

	spotLights.resize(1024);
	for (auto& light : spotLights)
	{
		light.position.x = Lerp(sceneMin.x, sceneMax.x, static_cast<float>(dist(gen)) / 1024.0f);
		light.position.y = Lerp(sceneMin.y, sceneMax.y, static_cast<float>(dist(gen)) / 1024.0f);
		light.position.z = Lerp(sceneMin.z, sceneMax.z, static_cast<float>(dist(gen)) / 1024.0f);

		light.radius = Lerp(10.0f, 300.0f, static_cast<float>(dist(gen)) / 1024.0f);

		light.color.r = Lerp(0.5f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);
		light.color.g = Lerp(0.5f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);
		light.color.b = Lerp(0.5f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);

		light.falloff = Lerp(1.0f, 2.0f, static_cast<float>(dist(gen)) / 1024.0f);

		light.direction.x = Lerp(-1.0f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
		light.direction.y = Lerp(-1.0f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
		light.direction.z = Lerp(-1.0f, 1.0f, static_cast<float>(dist(gen)) / 1024.0f);
		DirectX::XMStoreFloat3(&light.direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&light.direction)));

		light.cosHalfAngle = cosf(Lerp(15.0f, 35.0f, static_cast<float>(dist(gen)) / 1024.0f));
	}
}

void TiledLightingRenderer::Destroy()
{
	sceneRenderTarget.Destroy();
	sceneDepthStencilBuffer.Destroy();
	geometryPass.Destroy();
	lightPass.Destroy();
	objects.clear();
	pointLights.clear();
	spotLights.clear();
}

bool TiledLightingRenderer::Resize(ID3D11Device* device, unsigned int argScreenWidth, unsigned int argScreenHeight, unsigned numSubSamples)
{
	sceneRenderTarget.Destroy();
	if (!sceneRenderTarget.Create(
		device,
		DXGI_FORMAT_R11G11B10_FLOAT,
		GPUResourceBindFlags::RenderTargetBit | GPUResourceBindFlags::ShaderResourceBit,
		argScreenWidth,
		argScreenHeight,
		1,
		numSubSamples))
		return false;

	sceneDepthStencilBuffer.Destroy();
	if (!sceneDepthStencilBuffer.Create(device,
		DepthStencilBuffer::Format::Depth24_Stencil8,
		argScreenWidth,
		argScreenHeight,
		numSubSamples))
		return false;


	if (!geometryPass.Resize(device, argScreenWidth, argScreenHeight, numSubSamples))
		return false;

	if (!lightPass.Resize(device, argScreenWidth, argScreenHeight, numSubSamples))
		return false;

	screenWidth = argScreenWidth;
	screenHeight = argScreenHeight;

	return true;
}

void TiledLightingRenderer::Render(ID3D11DeviceContext* deviceContext, const CBaseCamera& camera)
{
	ViewInfo viewInfo;

	viewInfo.viewMatrix = camera.GetViewMatrix();
	viewInfo.projectionMatrix = camera.GetProjMatrix();
	viewInfo.viewProjectionMatrix = viewInfo.viewMatrix * viewInfo.projectionMatrix;
	viewInfo.invViewMatrix = DirectX::XMMatrixInverse(nullptr, viewInfo.invViewMatrix);
	viewInfo.invProjectionMatrix = DirectX::XMMatrixInverse(nullptr, viewInfo.invProjectionMatrix);
	viewInfo.invViewProjectionMatrix = DirectX::XMMatrixInverse(nullptr, viewInfo.invViewProjectionMatrix);
	DirectX::XMStoreFloat3(&viewInfo.viewOrigin, camera.GetEyePt());
	viewInfo.viewWidth = screenWidth;
	viewInfo.viewHeight = screenHeight;

	sceneDepthStencilBuffer.Clear(deviceContext, 0.0f, 0);
	geometryPass.Render(deviceContext, sceneDepthStencilBuffer, objects, viewInfo);
	lightPass.Render(deviceContext, sceneDepthStencilBuffer, geometryPass, pointLights, spotLights, viewInfo);
}

void TiledLightingRenderer::AddObject(const std::shared_ptr<Object>& object)
{
	if (std::find(objects.begin(), objects.end(), object) == objects.end())
		objects.push_back(object);
}

void TiledLightingRenderer::RemoveObject(const std::shared_ptr<Object>& object)
{
	objects.remove(object);
}