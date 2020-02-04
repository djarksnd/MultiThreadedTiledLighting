# README
DirectX11의 DeferredContext를 이용한 MultiThreadedTiledLighting 구현입니다. 

<img src="https://github.com/djarksnd/MultiThreadedTiledLighting/blob/master/ScreenShot.png?raw=true" width=400 height=300> 

<img src="https://github.com/djarksnd/MultiThreadedTiledLighting/blob/master/AnimatedGIF.gif?raw=true" width=400 height=300> 

## Files
이 포트폴리오는 실행파일을 빌드 할 수 있는 프로젝트 파일과(Visual Studio 2017 & 2019) 빌드된 바이너리 파일을 포함합니다.
* `bin`  _실행 가능한 바이너리 파일 폴더_
    * `TiledLighting_x64_release.exe`  _실행 가능한 바이너리  파일(Windows 64bit only / 바이너리 파일 실행 시 추가 DLL이 필요할 수 있습니다.)_
* `dxut`  _DXUT프로젝트 폴더 (포트폴리오 빌드 시 필요)_
*  `media`  _포트폴리오 데이터 폴더(쉐이더 코드 & 그래픽 데이터)_
* `TiledLighting`  _C++ 소스코드 및 프로젝트 파일 폴더_
* `TiledLighting_VS2017_Win10.sln`  _Visual Studio 2017 솔루션 파일_
* `TiledLighting_VS2019_Win10.sln`  _Visual Studio 2019 솔루션 파일_

## Important Implementations  
* MultiThreadedRendering with DeferredContext
	* 구현 내용은 아래의 함수들을 보시면 됩니다.
		* TiledRenderer.cpp
			* TiledRenderer::Render
			* TiledRenderer::FlushRenderTasks
			* TiledRenderer::RenderingThreadProc
* TiledSorted Deferred Rendering
	* 구현 내용은 아래의 함수들을 보시면 됩니다.
		* LightPass.cpp
			* LightPass::Render
		* LightCullingComputeShader.hlsl
			* main
* One - pass Dynamic PointLight Shadow with GeometryShader and RenderTargetArray 
	* 구현 내용은 아래의 함수들을 보시면 됩니다.
		* ShadowDepthBuffer.cpp
			* ShadowDepthBuffer::RenderPointLightShadowDepth
		* ShadowDepthGeometryShader.hlsl
			* main
