// include the basic windows header files and the Direct3D header file
#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <windows.h>
#include <Dwmapi.h>
#include <VersionHelpers.h>
#include <mutex>

#include "Direct3DCap.h"
#include "ScreenCalc.h"
#include "SerialClass.h"
#include "GDICap.h"
#include "DXGI.h"



#pragma comment(lib, "Dwmapi.lib")


#define GDI_CAP 0
#define D3D_CAP 1
#define D11_CAP 2

#define Windows8 0
#define Windows7 1


void CreateConfig(std::ofstream &file, Direct3DCap &cap);
int *LedAmountTest(char *);
void send_data(Serial* , char * , ScreenCalc &,std::mutex *);
void calc(ScreenCalc &);
BOOL DisableAeroTheme();

UINT8 Thread_comm = 0x01;


int main()
{
	bool exit = false;						//dit is voor later een escape variable
	
	float gamma = 0.6;
	//define
	Direct3DCap *DX9;
	GDICap *GDI;
	DXGI *DX11;
	std::mutex mtx;

	//post welke schermen beschikbaar zijn
	DX9 = new Direct3DCap;


	int cap_method = D11_CAP;
	int OS = 0;
	if (IsWindows8OrGreater())
	{
		std::cout << "Windows 8 detected. For optimal performance use DX11" << std::endl;
		OS = Windows8;
		cap_method = D11_CAP;
	}
	else if (IsWindows7OrGreater())
	{
		std::cout << "Windows 7 detected." << std::endl;
		std::cout << "Aero will be disabled for performance reason!" << std::endl;
		DisableAeroTheme();
		OS = Windows7;
		cap_method = D11_CAP;
	}
	else
	{
		std::cout << "Unsopperted windows version detected closing software" << std::endl;
		return 0;
	}

	std::ifstream myinfile;
	myinfile.open("./Config.txt");


	//Als het bestand is geopend bestaat die al dus sla je het maken over
	if (!myinfile.is_open())
	{
		myinfile.close();
		std::ofstream myfile;
		myfile.open("./Config.txt");

		CreateConfig(myfile, *DX9);

		myfile.close();

		myinfile.open("./Config.txt");
	}


	std::string STRING;
	int Config[9] = { -1 };

	for (int i = 0; i < 9; i++)
	{
		if (myinfile.eof())
			break;
		std::getline(myinfile, STRING);
		Config[i] = atoi(STRING.c_str());
	}
	myinfile.close();

	//Check of config laden goed is gelukt
	int i = 0;
	while (Config[i] != -1 && i < 9)
	{
		i++;
	}
	if (i < 9)
	{
		std::cout << "Config Loading went wrong! Please delete Config.txt and run this software again!" << std::endl;
		return 0;
	}

	std::cout << "Using screen: " << Config[0] << " for capturing" << std::endl;


	UINT32 *pBits;
	switch (cap_method)
	{
	case GDI_CAP:
		GDI = new GDICap;
		GDI->init(Config[0]);
		pBits = GDI->pBits;
		break;
	case D3D_CAP:
		DX9 = new Direct3DCap;
		DX9->init(Config[0]);
		pBits = DX9->pBits;
		break;
	case D11_CAP:
		DX11 = new DXGI;
		DX11->init(Config[0]);
		pBits = DX11->pBits;
		break;
	}

	ScreenCalc Scherm(105,					//init de kleur bereken functies
		pBits,			//De PixelData
		DX9->return_hres(),	//De Hori Resolutie 
		DX9->return_vres(),	//De Verti Resolutie
		Config[1],					//Hoeveel procent die moet nemen aan de bovenkant/onderkant
		Config[2],					//Hoeveel procent die aan de zijkant moet nemen
		Config[3],				//Leds Boven
		Config[5],					//Leds Onder
		Config[4],					//Leds Links
		Config[6],				//Leds Rechts
		Config[7]
		);

	//deze is nu niet meer nodig
	delete DX9;
	DX9 = nullptr;

	Scherm.Bereken_Grid();					//stel de hoeveelheid leds in die worden gebruikt en bereken Grid Grootte

	Scherm.set_Gamma(gamma);

	//Het programma moet eerst 0xff binnen krijgen als dat het geval is dan mag die beginnen met het oversturen
	//van de hoeveelheid leds
	//Als die hoeveelheden overeenkomen mag die beginnen met het zenden van led data
	std::string String;
	String = "\\\\.\\COM";
	String += std::to_string(Config[8]);
	char *temp = new char[String.size() + 1];
	std::copy(String.begin(), String.end(), temp);
	temp[String.size()] = '\0';
	Serial* SP = new Serial(temp);

	

	delete[] temp;
	temp = nullptr;

	if (SP->IsConnected())
		std::cout << "Connected with COM5" << std::endl;
	else
	{
		std::cout << "Communication Error. Exiting" << std::endl;
		return 0;
	}

	char Rx_buffer[100] = "12345678";	//Dit is de Rx_buffer deze moet een char zijn

	//je kan hier op escape drukken om het programma af ste sluiten hier
	std::cout << "Waiting for Arduino. Press ESC to quit" << std::endl;

	SP->ReadData(Rx_buffer, 2);
	while (Rx_buffer[0] != '0') //blijf hier hangen tot de arduino klaar is
	{
		Sleep(100);
		SP->ReadData(Rx_buffer, 2);
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			exit = true;
		}
		if (exit == true)
		{
			std::cout << "Something went wrong with communication. Check baudrate settings and COM port" << std::endl;
			return 0;	//beeindig de software
		}
	}

	Rx_buffer[0] = '0';

	std::cout << "Got response from arduino sending amount of leds" << std::endl;

	//Stuur de hoeveelheid leds naar de arduino
	UINT8 Tx_buffer[600 * 3];

	ZeroMemory(Tx_buffer, 600 * 3);
	Tx_buffer[0] = Scherm.geefLeds() >> 8 & 0x00FF;
	Tx_buffer[1] = Scherm.geefLeds() & 0x00FF;

	SP->WriteData((char*)Tx_buffer, 2);

	Sleep(1000);	//Wacht 1 seconde op de arduino

	std::cout << "Press END to quit capturing" << std::endl;

	UINT8 *pointer=NULL;						//Pointer voor de led bitstream

	pointer = Scherm.GeefLedPointer();	//Koppel de led bitstream aan de pointer
	// maak een thread die nu nog niks doet
	std::thread *uart;
	uart = new std::thread(send_data,SP,Rx_buffer,Scherm,&mtx);

	while (exit == false)
	{
		
		if (GetAsyncKeyState(VK_END))						//Als escape is ingedrukt zet exit true
		{
			exit = true;
		}
		else if (GetAsyncKeyState(VK_F8)&OS==Windows7)
		{
			delete DX9;
			pBits = GDI->pBits;
			Scherm.set_data(pBits);
			cap_method = GDI_CAP;
			std::cout << "Changed capture method to GDI " << std::endl;
			Sleep(100);

		}
		else if (GetAsyncKeyState(VK_F9)&OS == Windows7)
		{
			delete GDI;
			pBits = DX9->pBits;
			Scherm.set_data(pBits);
			cap_method = D3D_CAP;
			std::cout << "Changed capture method to D3D " << std::endl;
			Sleep(100);
		}
		else if (GetAsyncKeyState(VK_F12))
		{
			gamma += 0.01;
			Scherm.set_Gamma(gamma);
		}
		else if (GetAsyncKeyState(VK_F11))
		{
			gamma -= 0.01;
			Scherm.set_Gamma(gamma);
		}
		else if (GetAsyncKeyState(VK_F10))
		{
			std::cout << "Gamma : " << gamma << std::endl;
		}


		//start een thread die de data stuurt

		//Maak screenshot en sla die op
		mtx.lock();
		switch (cap_method)
		{
		case GDI_CAP:
			GDI->capture();
			Scherm.Bereken();
			Scherm.Calc_Aspect_ratio();
			break;
		case D3D_CAP:
			DX9->capture();
			Scherm.Bereken();
			Scherm.Calc_Aspect_ratio();
			break;
		case D11_CAP:
			if (DX11->capture())
			{
				Scherm.Bereken();
				Scherm.Calc_Aspect_ratio();
			}
			break;
		}
		mtx.unlock();
		
		//send_data(SP, Rx_buffer, Scherm);
		//Scherm.Calc_Aspect_ratio();
		//wacht tot alle data verzonden is en we weer antwoord hebben gehad dat alles in orde is voordat we weer verder gaan
		
	}
	Thread_comm = 0;

	uart->join();

	return 0;
}


// Dit moet in een andere thread gebeuren
void send_data(Serial* SP, char * Rx_buffer, ScreenCalc &Scherm, std::mutex *mtx)
{
	clock_t klok23;
	while (Thread_comm == 0x01)
	{
		mtx->lock();
		SP->WriteData((char*)Scherm.GeefLedPointer(), Scherm.geefLeds() * 3);	//Stuur alle data weg
		mtx->unlock();
			klok23 = clock();
			SP->ReadData(Rx_buffer, 10);
			while (Rx_buffer[0] != '1' || ((clock() - klok23) / CLOCKS_PER_SEC) > (float)0.5)		//Wacht tot arduino weer klaar is
			{
				SP->ReadData(Rx_buffer, 100);
			}
			Rx_buffer[0] = '0';

	}
}


void CreateConfig(std::ofstream &file, Direct3DCap &cap)
{
	file.clear();
	//onderstaande code vraagt om scherm
	int i = 1;
	std::cout << "Choose one of the above screens" << std::endl;
	scanf("%d", &i);
	while (i >= cap.return_adapterCounnt() || i < 0)
	{
		std::cout << "Choice: " << i << " is invalid \nChoose one of the above screens" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	std::cout << "How many percent should be captured from the top/bottom (0-100)" << std::endl;
	scanf("%d", &i);
	while (i < 1 || i > 100)
	{
		std::cout << "Choice: " << i << " is invalid \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	std::cout << "How many percent should be captured from the left/right side (0-100)" << std::endl;
	scanf("%d", &i);
	while (i < 1 || i > 100)
	{
		std::cout << "Choice: " << i << " is invalid \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	int j = 0;
	std::cout << "Which COM port is the arduino on" << std::endl;
	scanf("%d", &j);
	while (j < 0 || j > 60)
	{
		std::cout << "Choice: " << i << " is invalid \n" << std::endl;
		scanf("%d", &j);
	}
	std::string String;
	String = "\\\\.\\COM";
	String += std::to_string(j);
	char *temp = new char[String.size() + 1];
	std::copy(String.begin(), String.end(), temp);
	temp[String.size()] = '\0';

	int *pointer = LedAmountTest(temp);

	file << pointer[0] << std::endl;
	file << pointer[1] << std::endl;
	file << pointer[2] << std::endl;
	file << pointer[3] << std::endl;

	i = 0;
	std::cout << "What is the minimum black treshold (0- 60)" << std::endl;
	scanf("%d", &i);
	while (i < 1 || i > 60)
	{
		std::cout << "Choice: " << i << " is invalid \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	
	file << j << std::endl;
}

int *LedAmountTest(char *Comm)
{
	Serial* SP = new Serial(Comm);

	if (SP->IsConnected())
		std::cout << "Connected with COM5" << std::endl;

	bool exit = false;
	//je kan hier op escape drukken om het programma af ste sluiten hier
	std::cout << "Waiting for Arduino. Press ESC to quit" << std::endl;
	char Rx_buffer[2] = "";

	while (Rx_buffer[0] != '0') //blijf hier hangen tot de arduino klaar is
	{
		Sleep(100);
		SP->ReadData(Rx_buffer, 2);
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			exit = true;
		}
		if (exit == true)
		{
			std::cout << "Something went wrong with communication. Check baudrate settings and COM port" << std::endl;
		}
	}

	Rx_buffer[0] = '0';

	std::cout << "Got response from arduino sending amount of leds" << std::endl;

	//Stuur de hoeveelheid leds naar de arduino
	UINT8 Tx_buffer[600 * 3];

	ZeroMemory(Tx_buffer, 600 * 3);
	Tx_buffer[0] = 600 >> 8 & 0x00FF;
	Tx_buffer[1] = 600 & 0x00FF;

	SP->WriteData((char*)Tx_buffer, 2);


	std::cout << "Press up to add a LED and press down to remove one." << std::endl;
	std::cout << "When you are satisfied press SPACEBAR to confirm" << std::endl;
	std::cout << "Press ESC to quit" << std::endl;

	//onderstaande stukje code zal blijven draaien tot je op ESC drukt
	static int leds[4] = { 0 }, i = 0;
	int offset = 0;
	leds[0]++;
	while (i <4)
	{
		if (GetAsyncKeyState(VK_SPACE))						//Als escape is ingedrukt zet exit true
		{
			offset += leds[i];
			i++;
			Sleep(500);
		}
		if (GetAsyncKeyState(VK_UP))
		{
			leds[i]++;
			ZeroMemory(Tx_buffer, 600 * 3);
			for (int j = 0; j < leds[i]; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					Tx_buffer[(j + offset) * 3 + k] = 0xff;
				}

			}

			SP->WriteData((char*)Tx_buffer, 1800);
			Sleep(50);
		}
		if (GetAsyncKeyState(VK_DOWN) && leds[i] > 1)
		{
			leds[i]--;
			ZeroMemory(Tx_buffer, 600 * 3);
			for (int j = 0; j < leds[i]; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					Tx_buffer[(j + offset) * 3 + k] = 0xff;
				}

			}
			SP->WriteData((char*)Tx_buffer, 1800);
			Sleep(50);
		}

	}
	SP->~Serial();
	return leds;
}

BOOL DisableAeroTheme()
{
	HRESULT hr = S_OK;
	BOOL isDwmCompositiondEnabled = FALSE;

	hr = DwmIsCompositionEnabled(&isDwmCompositiondEnabled);
	if (SUCCEEDED(hr))
	{
		//printf("isDwmCompositiondEnabled: %d\n", isDwmCompositiondEnabled);
	}
	else
	{
		printf("DwmIsCompositionEnabledFn is Failed (Or) DWM composition is not Enabled!, Error Code:%d\n", GetLastError());

	}
	//if DWM composition is Enabled, Disable DWM composition.
	if (isDwmCompositiondEnabled)
	{
		//Disable Desktop Window Manager (DWM) composition.
		hr = DwmEnableComposition(FALSE);
		if (SUCCEEDED(hr))
		{
			return TRUE;
		}
		else
		{
			printf("DwmEnableCompositionFn is Failed, Error Code:%d\n", GetLastError());
		}
	}
	else
	{
		printf("DWM composition is not Enabled\n");
		return FALSE;
	}
}
