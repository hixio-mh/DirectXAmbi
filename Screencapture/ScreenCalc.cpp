#include "ScreenCalc.h"
#include <iostream>

ScreenCalc::ScreenCalc(float Diago, UINT32 *DataSet, int hres, int vres, int BlockH, int BlockV, int ledspm) :Hres(hres), Vres(vres), LedData(NULL), BlockDepthHori(BlockH), BlockDepthVert(BlockV), Blok(NULL), LedsPm(ledspm)
{
	double verhouding;
	verhouding = (double)Hres / (double)Vres;
	Hoogte = sin(atan((double)1 / verhouding)) * Diago*2.54;
	Lengte = Hoogte * verhouding;
	PixelData = DataSet;
	LedAantal = ((int)Hoogte*LedsPm / 100) * 2 + ((int)Lengte*LedsPm / 100) * 2;
}

ScreenCalc::~ScreenCalc()
{
	//Ruim alles netjes op
	delete PixelData;
	delete LedData;

	PixelData = NULL;
	LedData = NULL;
	Blok = NULL;
}

void ScreenCalc::Bereken_Grid()
{
	//Als Leds ongelijk is aan NULL dan moet je deze verwijderen omdat er dan al eentje bestaat. En we willen geen Memory Leaks :D

	if (LedData != NULL)
	{
		delete LedData;
		LedData = NULL;
	}

	if (Blok != NULL)
	{
		delete Blok;
		Blok = NULL;
	}

	LedData = new UINT8[LedAantal * 3];
	Blok = new Grid[LedAantal];


	//Zet alles op 0
	ZeroMemory(LedData, LedAantal * 3);

	//GridSize berekeningen
	int i, j;
	int led = 0;
	//bovenste rij
	for (i = 0; i < (LedsPm*(int)Lengte / 100); i++)
	{
		Blok[led].TLX = (Hres*i) / (LedsPm*(int)Lengte / 100);	//
		Blok[led].TLY = 0;
		Blok[led].BRX = (int)(Hres*(i + 1)) / (LedsPm*(int)Lengte / 100);
		Blok[led].BRY = (Vres*BlockDepthVert) / 100;
		led++;
	}

	//rechter rij
	for (j = 0; j < (LedsPm*(int)Hoogte / 100); j++)
	{
		Blok[led].TLX = Hres - ((Hres * BlockDepthHori) / 100);
		Blok[led].TLY = (Vres*j) / (LedsPm*Hoogte / 100);
		Blok[led].BRX = Hres;
		Blok[led].BRY = (Vres*(j + 1)) / (LedsPm*Hoogte / 100);
		led++;
	}
	//onderste rij van links naar rechts
	for (i = 0; i < (LedsPm*(int)Lengte / 100); i++)
	{
		Blok[led].TLX = Hres - (Hres*(i + 1)) / (LedsPm*(int)Lengte / 100);	//
		Blok[led].TLY = Vres - ((Vres*BlockDepthVert) / 100);
		Blok[led].BRX = Hres - (Hres*i) / (LedsPm*(int)Lengte / 100);
		Blok[led].BRY = Vres;
		led++;
	}
	//linker rij onder naar boven
	for (j = 0; j < (LedsPm*(int)Hoogte / 100); j++)
	{
		Blok[led].TLX = 0;
		Blok[led].TLY = Vres - (Vres*(j + 1)) / (LedsPm*(int)Hoogte / 100);
		Blok[led].BRX = (Hres*BlockDepthVert) / 100;
		Blok[led].BRY = Vres - (Vres*j) / (LedsPm*(int)Hoogte / 100);
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
			if ((((PixelData[x + y*Hres] >> 0) & 0xFF) < 0x20) && (((PixelData[x + y*Hres] >> 8) & 0xFF) < 0x20) && (((PixelData[x + y*Hres] >> 16) & 0xFF) < 0x20))
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

	Led[0] = g / j;
	Led[1] = r / j;
	Led[2] = b / j;
	
}

UINT16 ScreenCalc::geefLeds()
{
	return LedAantal;
}

UINT8 *ScreenCalc::GeefLedPointer()
{
	return LedData;
}