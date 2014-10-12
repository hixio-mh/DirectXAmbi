#pragma once

#include <Windows.h>
#include <math.h>

typedef struct Grid {
	int TLX = 0;
	int TLY = 0;
	int BRX = 0;
	int BRY = 0;
};

class ScreenCalc
{
public:
	ScreenCalc(float Diago, UINT32 *DataSet, int Hres, int Vres, int BlockH, int BlockV,
				int boven, int onder, int links, int rechts, int Black);
	~ScreenCalc();
	void Bereken_Grid();
	
	void set_Gamma(float Gamma);
	void Bereken();

	void SetOffset(int *Offset);
	void set_data(UINT32 *dataset);

	float Calc_Aspect_ratio();

	UINT16 geefLeds();
	UINT8 *GeefLedPointer();

private:
	void Gemiddelde(UINT8 *Led, int TopLeftX, int TopLeftY, int BottomLeftY, int BottomRightX);

	float Hoogte;
	float Lengte;
	
	int Hres, Vres;
	int BlockDepthHori;	//in procent
	int BlockDepthVert; //in procent
	int LedsBoven;
	int LedsOnder;
	int LedsLinks;
	int LedsRechts;
	int BlackLevel;

	UINT16 LedAantal;

	UINT32 *PixelData;
	UINT8 *LedData;	//multidimensio pointer

	Grid *Blok;
	
	int *Offset;

	//Gamma correction lookup table
	int *GammaE;
};

