# README
DirectX11의 DeferredContext를 이용한 MultiThreadedTiledLighting 구현입니다. 

<img src="https://github.com/djarksnd/MultiThreadedTiledLighting/blob/master/ScreenShot.png?raw=true" width=400 height=300> 

<img src="https://github.com/djarksnd/MultiThreadedTiledLighting/blob/master/AnimatedGIF.gif?raw=true" width=400 height=300> 

## Important Implementations  
* MultiThreadedRendering with DeferredContext
* TiledSorted Deferred Rendering
* One - pass Dynamic PointLight Shadow with GeometryShader and RenderTargetArray 

## Files
이 포트폴리오는 실행파일을 빌드 할 수 있는 프로젝트 파일과(Visual Studio 2017 & 2019) 빌드된 바이너리 파일을 포함합니다.
* `bin`  _실행 가능한 바이너리 파일 폴더_
    * `TiledLighting_x64_release.exe`  _실행 가능한 바이너리  파일(Windows 64bit only / 바이너리 파일 실행 시 추가 DLL이 필요할 수 있습니다.)_
* `dxut`  _DXUT프로젝트 폴더 (포트폴리오 빌드 시 필요)_
*  `media`  _포트폴리오 데이터 폴더(쉐이더 코드 & 그래픽 데이터)_
* `TiledLighting`  _C++ 소스코드 및 프로젝트 파일 폴더_
* `TiledLighting_VS2017_Win10.sln`  _Visual Studio 2017 솔루션 파일_
* `TiledLighting_VS2019_Win10.sln`  _Visual Studio 2019 솔루션 파일_

## Download Release
* MultiThreadedTiledLighting.zip
	* https://github.com/djarksnd/MultiThreadedTiledLighting/releases/download/1.0/MultiThreadedTiledLighting.zip
