#include "TiledLighting.h"

bool TiledLighting::Create(ID3D11Device* device, unsigned int screenWidth, unsigned int screenHeight, unsigned numSubSamples)
{
	if (!depthStencilBuffer.Create(device,
		DepthStencilBuffer::Format::Depth24_Stencil8,
		screenWidth,
		screenHeight,
		numSubSamples))
		return false;

	if (!geometryPass.Create(device, screenWidth, screenHeight, numSubSamples))
		return false;

	return true;
}

void TiledLighting::Destroy()
{
	opaqueObjects.clear();
	translucentObjects.clear();

	depthStencilBuffer.Destroy();
	geometryPass.Destroy();
}

bool TiledLighting::Resize(ID3D11Device* device, unsigned int screenWidth, unsigned int screenHeight, unsigned numSubSamples)
{
	depthStencilBuffer.Destroy();
	if (!depthStencilBuffer.Create(device,
		DepthStencilBuffer::Format::Depth24_Stencil8,
		screenWidth,
		screenHeight,
		numSubSamples))
		return false;

	if (!geometryPass.Resize(device, screenWidth, screenHeight, numSubSamples))
		return false;

	return true;
}

void TiledLighting::Render(ID3D11DeviceContext* deviceContext,
						   const CBaseCamera& camera)
{
	depthStencilBuffer.Clear(deviceContext, 0.0f, 0);

	geometryPass.Render(deviceContext, depthStencilBuffer, camera, opaqueObjects, translucentObjects);
}

void TiledLighting::AddOpaqueObject(const std::shared_ptr<MeshObject>& object)
{
	if (std::find(opaqueObjects.begin(), opaqueObjects.end(), object) == opaqueObjects.end())
		opaqueObjects.push_back(object);
}

void TiledLighting::AddTranslucentObject(const std::shared_ptr<MeshObject>& object)
{
	if (std::find(translucentObjects.begin(), translucentObjects.end(), object) == translucentObjects.end())
		translucentObjects.push_back(object);
}

void TiledLighting::RemoveOpaqueObject(const std::shared_ptr<MeshObject>& object)
{
	opaqueObjects.remove(object);
}

void TiledLighting::RemoveTranslucentObject(const std::shared_ptr<MeshObject>& object)
{
	translucentObjects.remove(object);
}