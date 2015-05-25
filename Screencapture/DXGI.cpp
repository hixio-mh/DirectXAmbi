#include "DXGI.h"

DXGI::DXGI()
{

}

DXGI::~DXGI()
{
	if (!DeskDupl)
	{
		DeskDupl->Release();
	}
	ReleaseFrame();
	if (dev != 0 &devcon != 0)
	{
		devcon->Release();
		dev->Release();
	}
	delete[] pBits;
	pBits = nullptr;
}


void DXGI::init(UINT IntNumber)
{

	// Define temporary pointers to a device and a device context

	// Create the device and device context objects
	D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&dev,
		nullptr,
		&devcon);
	initdup(IntNumber);

}


void DXGI::initdup(UINT IntNumber)
{
	//duplication interfaces
	// Get DXGI device

	IDXGIDevice* DxgiDevice = nullptr;

	dev->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
		
	// Get DXGI adapter
	IDXGIAdapter* DxgiAdapter = nullptr;
	DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
	DxgiDevice->Release();
	DxgiDevice = nullptr;
	
	// Get output
	IDXGIOutput* DxgiOutput = nullptr;
	DxgiAdapter->EnumOutputs(IntNumber, &DxgiOutput);		//de in moet een int zijn die staat voor The index of the output.
	DxgiAdapter->Release();
	DxgiAdapter = nullptr;

	//Get output description
	DxgiOutput->GetDesc(&OutputDesc);

	//create the duplication interface
	IDXGIOutput1* DxgiOutput1 = nullptr;
	DxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&DxgiOutput1);
	DxgiOutput->Release();
	DxgiOutput = nullptr;

	//create the duplicator
	DxgiOutput1->DuplicateOutput(dev, &DeskDupl);
	DxgiOutput1->Release();
	DxgiOutput1 = nullptr;

	DXGI_OUTDUPL_DESC desc;
	DeskDupl->GetDesc(&desc);
	std::cout << "Screen:" << IntNumber << " " << desc.ModeDesc.Width << "X" << desc.ModeDesc.Height << " is selected" << std::endl;
	
	pBits = new UINT32[desc.ModeDesc.Width*desc.ModeDesc.Height];
}


void DXGI::capture()
{

	ReleaseFrame();

	
	
	GetFrame();
	
	
	if (!CPUTexture)
	{
		AcquiredDesktopImage->GetDesc(&textDesc);
		textDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textDesc.Usage = D3D11_USAGE_STAGING;
		textDesc.BindFlags = 0;
		textDesc.MiscFlags = 0;
		dev->CreateTexture2D(&textDesc, NULL, &CPUTexture);
	}
	

	devcon->CopyResource(CPUTexture, AcquiredDesktopImage);

	devcon->Map(CPUTexture, 0, D3D11_MAP_READ, NULL, &pMappedData);

	BYTE* mappedData = reinterpret_cast<BYTE*>(pMappedData.pData);

	#define BITSPERPIXEL 32

	for (int i = 0; i < textDesc.Height; i++)
	{
		memcpy((BYTE*)pBits + i * textDesc.Width * BITSPERPIXEL / 8,
			mappedData + i* pMappedData.RowPitch,
			textDesc.Width * BITSPERPIXEL / 8);
	}

}

void DXGI::ReleaseFrame()
{
	DeskDupl->ReleaseFrame();

	if (AcquiredDesktopImage)
	{
		AcquiredDesktopImage->Release();
		AcquiredDesktopImage = nullptr;
	}
}

void DXGI::GetFrame()
{
	IDXGIResource* DesktopResource = nullptr;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	//haal het volgende frame binnen
	HRESULT hr = DeskDupl->AcquireNextFrame(500, &FrameInfo, &DesktopResource);
	if (FAILED(hr))
	{
		
		if (hr == 0x887A0027)
		{
			std::cout << "Time out" << std::endl;
		}
		else
		{
			std::cout << "acquire frame failed with" << std::hex << hr << std::endl;
		}
		return;
	}

	if (AcquiredDesktopImage)
	{
		AcquiredDesktopImage->Release();
		AcquiredDesktopImage = nullptr;
	}


	//schrijf deze weg naar een 2d texture en release daarna de resource weer
	DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&AcquiredDesktopImage));
	DesktopResource->Release();
	DesktopResource = nullptr;
}