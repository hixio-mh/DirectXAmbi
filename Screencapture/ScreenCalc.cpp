#include "ScreenCalc.h"
#include <iostream>

ScreenCalc::ScreenCalc(float Diago, UINT32 *DataSet, int hres, int vres, int BlockV, 
						int BlockH, int boven, int onder, int links, int rechts, int Black) 
:Hres(hres), Vres(vres), LedData(NULL), BlockDepthHori(BlockH), BlockDepthVert(BlockV), Blok(NULL), 
LedsBoven(boven), LedsOnder(onder), LedsLinks(links), LedsRechts(rechts), BlackLevel(Black), 
GammaE(NULL), OldLedData(NULL), K_P(0.8)
{
	double verhouding;
	verhouding = (double)Hres / (double)Vres;
	Hoogte = sin(atan((double)1 / verhouding)) * Diago*2.54;
	Lengte = Hoogte * verhouding;
	PixelData = DataSet;
	LedAantal = LedsBoven + LedsLinks + LedsRechts + LedsOnder;
	OldLedData = new UINT8[LedAantal*3];	//Voor PID
	ZeroMemory(OldLedData, LedAantal * 3);
	GammaE = new int[256] {0};
	Offset = new int[8] {0};
	set_Brightness(BlackLevel);
}

ScreenCalc::~ScreenCalc()
{
	//Ruim alles netjes op
/*	delete[] LedData;
	delete[] GammaE;
	delete[] Blok;

	GammaE = nullptr;
	PixelData = nullptr;
	LedData = nullptr;
	Blok = nullptr;
*/
}

void ScreenCalc::set_data(UINT32 *dataset)
{
	PixelData = dataset;
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
	int deltaB,deltaG,deltaR;
	for (int i = 0; i < LedAantal; i++)
	{
		//Dit zou nog in een aggressieve PID gedaan kunnen worden om een vloeiender effect te kunnen krijgen
		
		Gemiddelde(LedData + (i * 3), Blok[i].TLX, Blok[i].TLY, Blok[i].BRY, Blok[i].BRX);
		
		deltaR = LedData[i * 3] - OldLedData[i * 3];
		deltaG = LedData[i * 3 + 1] - OldLedData[i * 3 + 1];
		deltaB = LedData[i * 3 + 2] - OldLedData[i * 3 + 2];

		LedData[i * 3] = OldLedData[i * 3] + deltaR * K_P;
		LedData[i * 3 + 1] = OldLedData[i * 3 + 1] + deltaG * K_P;
		LedData[i * 3 + 2] = OldLedData[i * 3 + 2] + deltaB * K_P;

		OldLedData[i * 3] = LedData[i * 3];
		OldLedData[i * 3 + 1] = LedData[i * 3 + 1];
		OldLedData[i * 3 + 2] = LedData[i * 3 + 2];
	}

}


void ScreenCalc::Gemiddelde(UINT8 *Led, int TopLeftX, int TopLeftY, int BottomRightY, int BottomRightX)
{
	int j = 0;
	int r = 0, g = 0, b = 0;
	int y, x;
	
	//zijkant correctie
	if (TopLeftX < hborder)
	{
		//bereken het verschil
		x = hborder - TopLeftX;
		TopLeftX = hborder;
		BottomRightX += x;
	}
	
	if (Hres - hborder < BottomRightX)
	{
		//bereken het verschil
		x = BottomRightX - (Hres - hborder);
		BottomRightX = Hres - hborder;
		TopLeftX -= x;
	}

	//bovenste rand correctie
	if (TopLeftY < vborder)
	{
		//bereken het verschil
		y = vborder - TopLeftY;
		TopLeftY = vborder;
		BottomRightY += y;
	}

	if (Vres - vborder < BottomRightY)
	{
		//bereken het verschil
		y = BottomRightY - (Vres - vborder);
		BottomRightY = Vres - vborder;
		TopLeftY -= y;
		if (TopLeftY < 0)
			TopLeftY = 0;
	}
	
	for (x = TopLeftX; x < BottomRightX; x++)
	{
		for (y = TopLeftY; y < BottomRightY; y++)
		{
			//Als het bijna puur zwart is sla je hem over bij gemiddelde berekening
			if ((((PixelData[x + y*Hres] >> 0) & 0xFF) < BlackLevel) && 
				(((PixelData[x + y*Hres] >> 8) & 0xFF) < BlackLevel) && 
				(((PixelData[x + y*Hres] >> 16) & 0xFF) < BlackLevel))
			{
			}
			else
			{
				b += ((PixelData[x + y*Hres] >> 0) & 0xFF);
				g += ((PixelData[x + y*Hres] >> 8) & 0xFF);
				r += ((PixelData[x + y*Hres] >> 16) & 0xFF);
				j++;
				
				
			}
			
			
		}
	}



	if (j == 0)
		j = 1;

	g = g/ j;
	b = b/ j;
	r = r/j;

	if (g < BlackLevel)
		Led[0] = 0;
	else	
		Led[0] = GammaE[g - brightness];
		

	if (r < BlackLevel)
		Led[1] = 0;
	else
		Led[1] = GammaE[r-brightness];

	if (b < BlackLevel)
		Led[2] = 0;
	else
		Led[2] = GammaE[b- brightness];
	
}
void ScreenCalc::set_Brightness(int bri)
{
	if (bri < 0)
		bri = 0;
	else if (bri > BlackLevel)
		brightness = BlackLevel;
	else
		brightness = bri;
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
	//Deze functie creeert een gamma lookup table
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

void ScreenCalc::SethOffset(int offset)
{

		hOffset = offset;

}

void ScreenCalc::SetvOffset(int offset)
{

		vOffset = offset;

}

int ScreenCalc::Calc_Aspect_ratio()
{
	Calc_Top_border();
	Calc_Side_border();
	if (hborder < hOffset)
	{
		hborder = hOffset;
	}
	if (vborder < vOffset)
	{
		vborder = vOffset;
	}
	
	COORD topLeft = { 0, 12 };
	SetConsoleCursorPosition(console, topLeft);
	std::cout << "Vertical offset: " << vborder << "\t" << std::endl;
	std::cout << "Horizontal offset: " << hborder << "\t" << std::endl;

	return 0;
}

void ScreenCalc::Calc_Side_border()
{
	//Deze functie berekend of de afgespeelde video het scherm vult of niet.
	//en returned de breedte van de eventuele zwarte balk
	//Eerst word het contrast extreem opgeschroefd zodat alleen de pixels met de waarde 0 niet veranderen.
	//daarna zoekt deze een zwarte balk
	//deze functie werkt alleen als de video player niet pixelwaarde 0,0,0 als zwart gebruikt
	int x = 0, y = 0;
	int xmin = Hres;
	while (y < Vres)
	{
		while ((PixelData[(y * Hres) + x] & 0xFFFFFF) * 256 == 0)
		{
			x++;
			if (x == Hres)
			{
				y++;
				x = 0;
			}
		}
		
		if (x < xmin)
			xmin = x;

		y++;
		x = 0;
	}
	hborder = xmin;
}

void ScreenCalc::Calc_Top_border()
{
	//Deze functie berekend of de afgespeelde video het scherm vult of niet.
	//en returned de breedte van de eventuele zwarte balk
	//Eerst word het contrast extreem opgeschroefd zodat alleen de pixels met de waarde 0 niet veranderen.
	//daarna zoekt deze een zwarte balk
	//deze functie werkt alleen als de video player niet pixelwaarde 0,0,0 als zwart gebruikt
	int x = 0, y = 0;
	int ymin = Vres;
	while (x < Hres)
	{
		while ((PixelData[(y * Hres) + x] & 0xFFFFFF) * 256 == 0)
		{
			y++;
			if (y == Vres)
			{
				x++;
				y = 0;
			}
		}

		if (y < ymin)
			ymin = y;
		x++;
		y = 0;
	}
	vborder = ymin;
}