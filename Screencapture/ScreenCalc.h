#pragma once

#include <Windows.h>
#include <math.h>

typedef struct Grid {
	int TLX;
	int TLY;
	int BRX;
	int BRY;
};

class ScreenCalc
{
public:
	ScreenCalc(float Diago, UINT32 *DataSet, int Hres, int Vres, int BlockH, int BlockV, int ledspm);
	~ScreenCalc();
	void Bereken_Grid();
	UINT16 geefLeds();
	void Bereken();
	UINT8 *GeefLedPointer();

private:
	void Gemiddelde(UINT8 *Led, int TopLeftX, int TopLeftY, int BottomLeftY, int BottomRightX);

	float Hoogte;
	float Lengte;
	
	int Hres, Vres;
	int BlockDepthHori;	//in procent
	int BlockDepthVert; //in procent
	int LedsPm;
	UINT16 LedAantal;

	UINT32 *PixelData;
	UINT8 *LedData;	//multidimensio pointer

	Grid *Blok;
};
