#include "ScreenCalc.h"
#include <iostream>

ScreenCalc::ScreenCalc(float Diago, UINT32 *DataSet, int hres, int vres, int BlockH, 
						int BlockV, int boven, int onder, int links, int rechts, int Black) 
:Hres(hres), Vres(vres), LedData(NULL), BlockDepthHori(BlockH), BlockDepthVert(BlockV), Blok(NULL), 
LedsBoven(boven), LedsOnder(onder), LedsLinks(links), LedsRechts(rechts), BlackLevel(Black), GammaE(NULL)
{
	double verhouding;
	verhouding = (double)Hres / (double)Vres;
	Hoogte = sin(atan((double)1 / verhouding)) * Diago*2.54;
	Lengte = Hoogte * verhouding;
	PixelData = DataSet;
	LedAantal = LedsBoven + LedsLinks + LedsRechts + LedsOnder;
	GammaE = new int[256]
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
			2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
			5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
			10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
			17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
			25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
			37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
			51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
			69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
			90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
			115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
			144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
			177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
			215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255 };
}

ScreenCalc::~ScreenCalc()
{
	//Ruim alles netjes op
	delete[] LedData;
	delete[] GammaE;

	GammaE = nullptr;
	PixelData = nullptr;
	LedData = nullptr;
	Blok = nullptr;
}

void ScreenCalc::Bereken_Grid()
{
	//Als Leds ongelijk is aan NULL dan moet je deze verwijderen omdat er dan al eentje bestaat. En we willen geen Memory Leaks :D

	if (LedData != NULL)
	{
		delete[] LedData;
		LedData = nullptr;
	}

	if (Blok != NULL)
	{
		delete[] Blok;
		Blok = nullptr;
	}

	LedData = new UINT8[(LedAantal * 3)];
	Blok = new Grid[LedAantal];


	//Zet alles op 0
	ZeroMemory(LedData, LedAantal * 3);

	//GridSize berekeningen
	int i, j;
	int led = 0;
	//bovenste rij
	for (i = 0; i < LedsBoven; i++)
	{
		Blok[led].TLX = (Hres*i) / LedsBoven;	//
		Blok[led].TLY = 0;
		Blok[led].BRX = (int)(Hres*(i + 1)) / LedsBoven;
		Blok[led].BRY = (Vres*BlockDepthVert) / 100;
		led++;
	}

	//rechter rij
	for (j = 0; j < LedsRechts; j++)
	{
		Blok[led].TLX = Hres - ((Hres * BlockDepthHori) / 100);
		Blok[led].TLY = (Vres*j) / LedsRechts;
		Blok[led].BRX = Hres;
		Blok[led].BRY = (Vres*(j + 1)) / LedsRechts;
		led++;
	}
	//onderste rij van rechts naar links
	for (i = 0; i < LedsOnder; i++)
	{
		Blok[led].TLX = Hres - (Hres*(i + 1)) / LedsOnder;	//
		Blok[led].TLY = Vres - ((Vres*BlockDepthVert) / 100);
		Blok[led].BRX = Hres - (Hres*i) / LedsOnder;
		Blok[led].BRY = Vres;
		led++;
	}
	//linker rij onder naar boven
	for (j = 0; j < LedsLinks; j++)
	{
		Blok[led].TLX = 0;
		Blok[led].TLY = Vres - (Vres*(j + 1)) / LedsLinks;
		Blok[led].BRX = (Hres*BlockDepthVert) / 100;
		Blok[led].BRY = Vres - (Vres*j) / LedsLinks;
		led++;
	}
}

void ScreenCalc::Bereken()
{
	for (int i = 0; i < LedAantal; i++)
	{
		Gemiddelde(LedData + (i * 3), Blok[i].TLX, Blok[i].TLY, Blok[i].BRY, Blok[i].BRX);
	}

}

void ScreenCalc::Gemiddelde(UINT8 *Led, int TopLeftX, int TopLeftY, int BottomRightY, int BottomRightX)
{
	int j = 0;
	int r = 0, g = 0, b = 0;
	int y, x;

	for (x = TopLeftX; x < BottomRightX; x++)
	{
		for (y = TopLeftY; y < BottomRightY; y++)
		{
			//Als het bijna puur zwart is sla je hem over bij gemiddelde berekening
			//std::cout << (unsigned int)(((PixelData[x + y*Hres] >> 0) & 0xFF)) << '/' << j << std::endl;
			if ((((PixelData[x + y*Hres] >> 0) & 0xFF) < BlackLevel) && (((PixelData[x + y*Hres] >> 8) & 0xFF) < BlackLevel) && (((PixelData[x + y*Hres] >> 16) & 0xFF) < BlackLevel))
			{
			}
			else
			{
				//std::cout << (PixelData[x + y*Hres]&0x000000FF) << ',' << ((PixelData[x + y*Hres] >> 0) & 0xFF) << std::endl;
				//Sleep(100);
				b += ((PixelData[x + y*Hres] >> 0) & 0xFF);
				g += ((PixelData[x + y*Hres] >> 8) & 0xFF);
				r += ((PixelData[x + y*Hres] >> 16) & 0xFF);
				j++;
				
			}
			
		}
	}
	if (j == 0)
		j = 1;
	//std::cout << r << '/' << temp << std::endl;

	Led[0] = GammaE[(g / j)];
	Led[1] = GammaE[(r / j)];
	Led[2] = GammaE[(b / j)];
	
}

UINT16 ScreenCalc::geefLeds()
{
	return LedAantal;
}

UINT8 *ScreenCalc::GeefLedPointer()
{
	return LedData;
}