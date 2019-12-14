#pragma once

#include <d3d11.h>
#include <list>
#include <memory>

#include "DepthStencilBuffer.h"
#include "RenderTarget.h"
#include "GeometryPass.h"
#include "LightPass.h"

class Object;
struct ViewInfo;

class TiledLightingRenderer
{
public:
	enum class StenCilMask
	{
		DeferredLightable,
	};

public:
	TiledLightingRenderer() : screenWidth(0), screenHeight(0) {}

	bool Create(ID3D11Device* device, unsigned int screenWidth, unsigned int screenHeight, unsigned numSubSamples);
	void Destroy();
	bool Resize(ID3D11Device* device, unsigned int screenWidth, unsigned int screenHeight, unsigned numSubSamples);
	void Render(ID3D11DeviceContext* deviceContext, const CBaseCamera& camera);

	static TiledLightingRenderer& GetInstance() 
	{
		static TiledLightingRenderer instance;
		return instance;
	}

	void AddObject(const std::shared_ptr<Object>& object);
	void RemoveObject(const std::shared_ptr<Object>& object);

private:
	void InitObjects(ID3D11Device* device);
	void InitLights();

private:
	unsigned int screenWidth;
	unsigned int screenHeight;
	RenderTarget sceneRenderTarget;
	DepthStencilBuffer sceneDepthStencilBuffer;
	GeometryPass geometryPass;
	LightPass lightPass;

	std::list<std::shared_ptr<Object>> objects;
	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;

};
