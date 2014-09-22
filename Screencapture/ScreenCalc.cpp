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
	GammaE = new int[256] {0};
	Offset = new int[8] {0};
}

ScreenCalc::~ScreenCalc()
{
	//Ruim alles netjes op
	delete[] LedData;
	delete[] GammaE;
	delete[] Blok;

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
	int led = Offset[0];
	//bovenste rij
	for (i = Offset[0]; i < LedsBoven - Offset[1]; i++)
	{
		Blok[led].TLX = (Hres*i) / LedsBoven;	//
		Blok[led].TLY = 0;
		Blok[led].BRX = (int)(Hres*(i + 1)) / LedsBoven;
		Blok[led].BRY = (Vres*BlockDepthVert) / 100;
		led++;
	}
	led += Offset[1];
	led += Offset[2];
	//rechter rij
	for (j = Offset[2]; j < LedsRechts - Offset[3]; j++)
	{
		Blok[led].TLX = Hres - ((Hres * BlockDepthHori) / 100);
		Blok[led].TLY = (Vres*j) / LedsRechts;
		Blok[led].BRX = Hres;
		Blok[led].BRY = (Vres*(j + 1)) / LedsRechts;
		led++;
	}
	led += Offset[3];
	led += Offset[4];
	//onderste rij van rechts naar links
	for (i = Offset[4]; i < LedsOnder-Offset[5]; i++)
	{
		Blok[led].TLX = Hres - (Hres*(i + 1)) / LedsOnder;	//
		Blok[led].TLY = Vres - ((Vres*BlockDepthVert) / 100);
		Blok[led].BRX = Hres - (Hres*i) / LedsOnder;
		Blok[led].BRY = Vres;
		led++;
	}
	led += Offset[5];
	led += Offset[6];
	//linker rij onder naar boven
	for (j = Offset[6]; j < LedsLinks-Offset[7]; j++)
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

void ScreenCalc::set_Gamma(float Gamma)
{
	for (int i = 0; i < 256; i++)
	{
		GammaE[i] = ((float)pow((float)((float)i / 255), (float)((float)1 / Gamma))) * (float)255;
		if (GammaE[i] > 255)
		{
			GammaE[i] = 255;
		}
		else if (GammaE[i] < 0)
		{
			GammaE[i] = 0;
		}
	}
}

void ScreenCalc::SetOffset(int *offset)
{
	for (int i = 0; i < 8; i++)
	{
		Offset[i] = offset[i];
	}
}

float ScreenCalc::Calc_Aspect_ratio()
{
	//Deze functie berekend of de afgespeelde video het scherm vult of niet.
	//Op het moment zoekt die een zwarte balk.
	//Deze methode werkt niet
	int i = 0;
	int j = 0;
	while (((PixelData[(j*Hres) + i] >> 16) & 0xFF) < 0x05 && ((PixelData[(j*Hres) + i] >> 8) & 0xFF) < 0x05 && ((PixelData[(j*Hres) + i]) & 0xFF) < 0x05)
	{
		if (i > Vres)
		{
			j++;
			i = 0;
		}
		else
			i++;
	}
	return 0;
}