#include "Direct3DCap.h"


Direct3DCap::Direct3DCap() :scherm_nummer(0)
{
	Direct3D = Direct3DCreate9(D3D_SDK_VERSION);
	//vraag de hoeveelheid adapters (schermen) op
	AdapterCount = Direct3D->GetAdapterCount();

	for (int i = 0; i < AdapterCount; i++)
	{
		if (FAILED(Direct3D->GetAdapterDisplayMode(i, &ddm)))
		{
			std::cout << "Unable to Get Adapter Display Mode" << std::endl;
		}
		//print alle schermen uit met hun bijbehorende resolutie
		std::cout << "Screen:" << i << " " << ddm.Width << "X" << ddm.Height << std::endl;
	}
}


Direct3DCap::~Direct3DCap()
{
	Surface->UnlockRect();
	Surface->Release();
	delete[] pBits;
	pBits = nullptr;
	
}

void Direct3DCap::init(UINT8 schermnummer)
{		
	if (FAILED(Direct3D->GetAdapterDisplayMode(schermnummer, &ddm)))
	{
		std::cout << "Unable to Get Adapter Display Mode" << std::endl;
	}

	D3DPRESENT_PARAMETERS PresParams;

	ZeroMemory(&PresParams, sizeof(PresParams));
	/*
	PresParams.Windowed = true;
	PresParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	PresParams.BackBufferFormat = ddm.Format;
	PresParams.BackBufferHeight = ddm.Height;
	PresParams.BackBufferWidth = ddm.Width;
	PresParams.BackBufferCount = 2;
	PresParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	PresParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	PresParams.hDeviceWindow = NULL;*/
	//PresParams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	PresParams.Windowed = true;
	PresParams.SwapEffect = D3DSWAPEFFECT_COPY;
	PresParams.BackBufferCount = 1;
	PresParams.BackBufferFormat = ddm.Format;
	PresParams.BackBufferHeight = ddm.Height;
	PresParams.BackBufferWidth = ddm.Width;
	PresParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;


	Direct3D->CreateDevice(schermnummer, D3DDEVTYPE_HAL,
		NULL, D3DCREATE_MIXED_VERTEXPROCESSING, &PresParams,
		&Direct3dDevice);

//
	Direct3dDevice->GetRenderTarget(0, &pRenderTarget);
//

	if (FAILED(Direct3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height,
		D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM,
		&Surface, NULL)))
	{
		std::cout << "Surface create error" << std::endl;
	}
	//maak een array aan waar het scherm in word opgeslagen
	pBits = new UINT32[ddm.Width*ddm.Height];
	
}

void Direct3DCap::capture()
{
	Direct3dDevice->GetFrontBufferData(0, Surface);
	//Direct3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &Surface);
	//Direct3dDevice->GetRenderTargetData(pRenderTarget, Surface);
	/*switch (Direct3dDevice->GetRenderTargetData(pRenderTarget, Surface))
	{
	case D3DERR_INVALIDCALL:
		std::cout << "D3DERR_INVALIDCALL";
		break;
	case D3DERR_DEVICELOST:
		std::cout << "D3DERR_DEVICELOST";
		break;
	case D3DERR_DRIVERINTERNALERROR:
		std::cout << "D3DERR_DRIVERINTERNALERROR";
		break;
	case D3D_OK:
		std::cout << "D3D_OK";
		break;
	default:
		std::cout << "different error: " << (Direct3dDevice->GetRenderTargetData(pRenderTarget, Surface));
	}
	std::cout << std::endl;*/
	

	D3DLOCKED_RECT	lockedRect;
	if (Surface->LockRect(&lockedRect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY) != D3D_OK)
	{
		std::cout << "D3D error" << std::endl;
	}
	
	for (int i = 0; i < ddm.Height; i++)
	{
		memcpy((BYTE*)pBits + i * ddm.Width * BITSPERPIXEL / 8,
			(BYTE*)lockedRect.pBits + i* lockedRect.Pitch,
			ddm.Width * BITSPERPIXEL / 8);
	}
	Surface->UnlockRect();
}

int Direct3DCap::return_vres()
{
	return ddm.Height;
}

int Direct3DCap::return_hres()
{
	return ddm.Width;
}

int Direct3DCap::return_adapterCounnt()
{
	return (int)AdapterCount;
}