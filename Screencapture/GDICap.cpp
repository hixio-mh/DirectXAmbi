#include "GDICap.h"


GDICap::GDICap()
{
	hDesktopWnd = GetDesktopWindow();
	GetWindowRect(hDesktopWnd, &screenInfo);

	dcDesktop = GetDC(hDesktopWnd);
	dcCapture = CreateCompatibleDC(dcDesktop);
	
	hCaptureBMP = CreateCompatibleBitmap(dcDesktop, screenInfo.right, screenInfo.bottom);
	SelectObject(dcCapture, hCaptureBMP);
	pBits = new UINT32[screenInfo.right*screenInfo.bottom];
	SelectObject(dcCapture, hCaptureBMP);
}


GDICap::~GDICap()
{
	DeleteDC(dcCapture);
	DeleteObject(hCaptureBMP);
}

void GDICap::init(UINT8 SCherm)
{
	//Doe hier niks want er is geen init nodig voor GDI
}

void GDICap::capture()
{
	
	BitBlt(dcCapture, 0, 0, screenInfo.right, screenInfo.bottom, dcDesktop, 0, 0, SRCCOPY | CAPTUREBLT);
	GetBitmapBits(hCaptureBMP, screenInfo.bottom*screenInfo.right*4, pBits);
}

int GDICap::return_hres()
{
	return screenInfo.right;
}

int GDICap::return_vres()
{
	return screenInfo.bottom;
}