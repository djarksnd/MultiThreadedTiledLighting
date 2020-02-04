# README
DirectX11의 [DeferredContext](https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread-render)를 이용한 MultiThreadedTiledLighting 구현입니다. 
다수의 조명과 그림자를 표현합니다.

<img src="https://github.com/djarksnd/MultiThreadedTiledLighting/blob/master/AnimatedGIF.gif?raw=true" width=400 height=300> 

## Important Implementations  
* [MultiThreadedRendering](https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread-intro) with [DeferredContext](https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread-render)
* [TiledSorted Deferred Rendering](https://github.com/GPUOpen-LibrariesAndSDKs/TiledLighting11/blob/master/tiledlighting11/doc/TiledLighting11.pdf)
* One-pass pointLight shadow depth drawing with [GeometryShader and RenderTargetArray](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus) 

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
* 빌드과정 없이 포트폴리오를 실행하려면 아래 파일을 내려받으시면 됩니다.
	* MultiThreadedTiledLighting.zip 
		* https://github.com/djarksnd/MultiThreadedTiledLighting/releases/download/1.0/MultiThreadedTiledLighting.zip
