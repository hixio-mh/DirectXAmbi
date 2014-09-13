#pragma once

#include <iostream>
#include <d3d9.h>
#include <string>
#include <sstream>

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")

#define BITSPERPIXEL 32

//Direct3D pointers


class Direct3DCap
{
public:
	Direct3DCap();
	~Direct3DCap();

	void init(UINT8 schermnummer);
	void capture();
	int return_hres();
	int return_vres();

public:
	IDirect3D9* Direct3D = NULL;
	IDirect3DDevice9* Direct3dDevice = NULL;
	IDirect3DSurface9* Surface = NULL;

	D3DLOCKED_RECT lockedRect;
	D3DDISPLAYMODE	ddm;

	UINT32	*pBits = NULL;

private:
	UINT8 AdapterCount;
	UINT8 scherm_nummer;
};

