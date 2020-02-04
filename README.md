# README
DirectX11�� DeferredContext�� �̿��� MultiThreadedTiledLighting �����Դϴ�. 

<img src="https://github.com/djarksnd/MultiThreadedTiledLighting/blob/master/ScreenShot.png?raw=true" width=400 height=300> 

<img src="https://github.com/djarksnd/MultiThreadedTiledLighting/blob/master/AnimatedGIF.gif?raw=true" width=400 height=300> 

## Files
�� ��Ʈ�������� ���������� ���� �� �� �ִ� ������Ʈ ���ϰ�(Visual Studio 2017 & 2019) ����� ���̳ʸ� ������ �����մϴ�.
* `bin`  _���� ������ ���̳ʸ� ���� ����_
    * `TiledLighting_x64_release.exe`  _���� ������ ���̳ʸ�  ����(Windows 64bit only / ���̳ʸ� ���� ���� �� �߰� DLL�� �ʿ��� �� �ֽ��ϴ�.)_
* `dxut`  _DXUT������Ʈ ���� (��Ʈ������ ���� �� �ʿ�)_
*  `media`  _��Ʈ������ ������ ����(���̴� �ڵ� & �׷��� ������)_
* `TiledLighting`  _C++ �ҽ��ڵ� �� ������Ʈ ���� ����_
* `TiledLighting_VS2017_Win10.sln`  _Visual Studio 2017 �ַ�� ����_
* `TiledLighting_VS2019_Win10.sln`  _Visual Studio 2019 �ַ�� ����_

## Important Implementations  
* MultiThreadedRendering with DeferredContext
	* ���� ������ �Ʒ��� �Լ����� ���ø� �˴ϴ�.
		* TiledRenderer.cpp
			* TiledRenderer::Render
			* TiledRenderer::FlushRenderTasks
			* TiledRenderer::RenderingThreadProc
* TiledSorted Deferred Rendering
	* ���� ������ �Ʒ��� �Լ����� ���ø� �˴ϴ�.
		* LightPass.cpp
			* LightPass::Render
		* LightCullingComputeShader.hlsl
			* main
* One - pass Dynamic PointLight Shadow with GeometryShader and RenderTargetArray 
	* ���� ������ �Ʒ��� �Լ����� ���ø� �˴ϴ�.
		* ShadowDepthBuffer.cpp
			* ShadowDepthBuffer::RenderPointLightShadowDepth
		* ShadowDepthGeometryShader.hlsl
			* main
