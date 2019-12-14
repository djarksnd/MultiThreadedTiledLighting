#pragma once

#include <d3d11.h>
#include <list>
#include <memory>

#include "DepthStencilBuffer.h"
#include "GeometryPass.h"

class MeshObject;
class CBaseCamera;

class TiledLighting
{
public:
	TiledLighting() {}

	bool Create(ID3D11Device* device, unsigned int screenWidth, unsigned int screenHeight, unsigned numSubSamples);
	void Destroy();
	bool Resize(ID3D11Device* device, unsigned int screenWidth, unsigned int screenHeight, unsigned numSubSamples);
	void Render(ID3D11DeviceContext* deviceContext,
				const CBaseCamera& camera);

	static TiledLighting& GetInstance() 
	{
		static TiledLighting instance;
		return instance;
	}

	void AddOpaqueObject(const std::shared_ptr<MeshObject>& object);
	void AddTranslucentObject(const std::shared_ptr<MeshObject>& object);
	void RemoveOpaqueObject(const std::shared_ptr<MeshObject>& object);
	void RemoveTranslucentObject(const std::shared_ptr<MeshObject>& object);

private:
	DepthStencilBuffer depthStencilBuffer;
	GeometryPass geometryPass;
	std::list<std::shared_ptr<MeshObject>> opaqueObjects;
	std::list<std::shared_ptr<MeshObject>> translucentObjects;
};
