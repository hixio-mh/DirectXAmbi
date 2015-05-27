#pragma once

#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <memory>
#include <iostream>

#pragma comment(lib, "D3D11.lib")

using namespace Microsoft::WRL;
using namespace DirectX;

class DXGI
{
public:
	DXGI();
	~DXGI();
	void init(UINT IntNumber);
	
	bool capture();
	

	UINT32	*pBits = NULL;

private:

	bool GetFrame();
	void initdup(UINT IntNumber);
	void ReleaseFrame();

	ID3D11Device *dev =NULL;              // the device interface
	ID3D11DeviceContext *devcon = NULL;    // the device context interface



	IDXGIOutputDuplication* DeskDupl = NULL;
	ID3D11Texture2D* AcquiredDesktopImage =NULL;

	DXGI_OUTPUT_DESC OutputDesc;

	ID3D11Texture2D *CPUTexture = NULL;
	D3D11_MAPPED_SUBRESOURCE pMappedData;
	
	D3D11_TEXTURE2D_DESC textDesc;
};
