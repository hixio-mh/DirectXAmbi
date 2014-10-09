#include "DX11Cap.h"


DX11Cap::DX11Cap()
{
	// this function initializes and prepares Direct3D for use
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	//scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_READ_ONLY;      // how swap chain is to be used
	//scd.SampleDesc.Count = 1;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	// create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);
}


DX11Cap::~DX11Cap()
{
	swapchain->Release();
	dev->Release();
	devcon->Release();
}

void DX11Cap::init()
{
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast< void** >(&pSurface));
	pSurface->GetDesc(&desc);
	if (pSurface)
	{
		pBits = new UINT32[desc.Width * desc.Height];

		ID3D11Texture2D* pNewTexture = NULL;

		D3D11_TEXTURE2D_DESC description;
		pSurface->GetDesc(&description);
		description.BindFlags = 0;
		description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		description.Usage = D3D11_USAGE_STAGING;
	}
}

void DX11Cap::capture()
{
	dev->CreateTexture2D(&desc, NULL, &pNewSurface);
	if (pNewSurface)
	{
		devcon->CopyResource(pNewSurface, pSurface);
		D3D11_MAPPED_SUBRESOURCE resource;
		unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
		devcon->Map(pNewSurface, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

		const int pitch = desc.Width << 2;

		for (int i = 0; i < desc.Height; i++)
		{
			memcpy((BYTE*)pBits + i * desc.Width * 32 / 8,
				(BYTE*)resource.pData + i* desc.Height,
				desc.Width * 32 / 8);
		}
	}
}