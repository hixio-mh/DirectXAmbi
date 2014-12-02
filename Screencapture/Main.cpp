// include the basic windows header files and the Direct3D header file
#include <iostream>
#include <fstream>
#include <ctime>
#include <windows.h>
#include <thread>

#include "Direct3DCap.h"
#include "ScreenCalc.h"
#include "SerialClass.h"
#include "GDICap.h"
#include "Direct3DCap.h"
#include "DX11Cap.h"

#define GDI_CAP 0
#define D3D_CAP 1
#define D11_CAP 2

void CreateConfig(std::ofstream &file, Direct3DCap &cap);
int *LedAmountTest(char *);
void send_data(Serial* , char * , ScreenCalc &, UINT8 *);

UINT8 Thread_comm = 0x01;


int main()
{
	bool exit = false;						//dit is voor later een escape variable
	int cap_method = D3D_CAP;
	float gamma = 0.6;

	GDICap Cap;
	Direct3DCap D3DCap;						//init directx9

	std::ifstream myinfile;
	myinfile.open("./Config.txt");


	//Als het bestand is geopend bestaat die al dus sla je het maken over
	if (!myinfile.is_open())
	{
		myinfile.close();
		std::ofstream myfile;
		myfile.open("./Config.txt");

		CreateConfig(myfile, D3DCap);

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
	D3DCap.init(Config[0]);
	Cap.init(Config[0]);							//

	UINT32 *pBits;
	switch (cap_method)
	{
	case GDI_CAP:
		pBits = Cap.pBits;
		break;
	case D3D_CAP:
		pBits = D3DCap.pBits;
		break;
	}

	ScreenCalc Scherm(105,					//init de kleur bereken functies
		pBits,			//De PixelData
		Cap.return_hres(),	//De Hori Resolutie 
		Cap.return_vres(),	//De Verti Resolutie
		Config[1],					//Hoeveel procent die moet nemen aan de bovenkant/onderkant
		Config[2],					//Hoeveel procent die aan de zijkant moet nemen
		Config[3],				//Leds Boven
		Config[5],					//Leds Onder
		Config[4],					//Leds Links
		Config[6],				//Leds Rechts
		Config[7]);

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

	char Rx_buffer[100] = "";	//Dit is de Rx_buffer deze moet een char zijn

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

	UINT8 *pointer;						//Pointer voor de led bitstream
	clock_t klok1;
	clock_t klok2 = 50;
	pointer = Scherm.GeefLedPointer();	//Koppel de led bitstream aan de pointer
	// maak een thread die nu nog niks doet
	std::thread uart(send_data,SP,Rx_buffer,Scherm,pointer);

	while (exit == false)
	{
		std::cout << "                     " << "\r";
		std::cout << ((clock() - klok2)) << "\r";
		//std::cout << ((1* CLOCKS_PER_SEC) / (clock() - klok2)) << "\r";
		klok2 = clock();

		if (GetAsyncKeyState(VK_END))						//Als escape is ingedrukt zet exit true
		{
			exit = true;
		}
		else if (GetAsyncKeyState(VK_HOME))
		{
			cap_method++;

			if (cap_method > 1)
				cap_method = 0;

			std::cout << "Changed capture method to: " << cap_method << std::endl;
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

//Scherm.Calc_Aspect_ratio();
		//Maak screenshot en sla die op

		switch (cap_method)
		{
		case GDI_CAP:
			pBits = Cap.pBits;
			Scherm.set_data(pBits);
			Cap.capture();
			break;
		case D3D_CAP:
			pBits = D3DCap.pBits;
			Scherm.set_data(pBits);
			D3DCap.capture();
			break;
		}
		Scherm.Bereken();				//Bereken alle led kleuren
		//wacht tot alle data verzonden is en we weer antwoord hebben gehad dat alles in orde is voordat we weer verder gaan
		
	}
	Thread_comm &= !0x01;
	uart.join();
	return 0;
}

// Dit moet in een andere thread gebeuren
void send_data(Serial* SP, char * Rx_buffer, ScreenCalc &Scherm, UINT8 *pointer)
{
	clock_t klok1;
	while (Thread_comm == 0x01)
	{
			SP->WriteData((char*)pointer, Scherm.geefLeds() * 3);	//Stuur alle data weg

			klok1 = clock();
			SP->ReadData(Rx_buffer, 10);
			while (Rx_buffer[0] != '1' || ((klok1 - clock()) / CLOCKS_PER_SEC) > (float)0.5)		//Wacht tot arduino weer klaar is
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
