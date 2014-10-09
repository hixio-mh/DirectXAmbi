#pragma once
#include <Windows.h>

class GDICap
{
public:
	GDICap();
	~GDICap();
	void capture();
	void init(UINT8 schermnummer);
	int return_hres();
	int return_vres();

	UINT32	*pBits = NULL;

private:
	HDC dcDesktop;
	HDC dcCapture;
	HWND hDesktopWnd;
	HBITMAP hCaptureBMP;
	BITMAP bmpDesktopCopy;
	RECT screenInfo;
};

