# README
DirectX11�� DeferredContext�� �̿��� MultiThreadedTiledLighting �����Դϴ�. 

<img src="https://github.com/djarksnd/MultiThreadedTiledLighting/blob/master/ScreenShot.png?raw=true" width=500 height=350> 

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
* TiledSorted Deferred Rendering
* Dynamic PointLight Shadow with GeometryShader and RenderTargetArray 