// include the basic windows header files and the Direct3D header file
#include <iostream>
#include <fstream>
#include <ctime>
#include <windows.h>

#include "Direct3DCap.h"
#include "ScreenCalc.h"
#include "SerialClass.h"

void CreateConfig(std::ofstream &file, Direct3DCap &cap);


int main()
{
	bool exit = false;						//dit is voor later een escape variable

	Direct3DCap Cap;						//init directx

	std::ifstream myinfile;
	myinfile.open("Config.txt");

	

	if (!myinfile.is_open())
	{
		myinfile.close();
		std::ofstream myfile;

		myfile.open("Config.txt");

		CreateConfig(myfile, Cap);

		myfile.close();

		myinfile.open("Config.txt");
	}

	std::string STRING;
	int Config[8];

	for (int i = 0; i < 8; i++)
	{
		if (myinfile.eof())
			break;
		std::getline(myinfile, STRING);
		Config[i] = atoi(STRING.c_str());
	}
	
	

	
	Cap.init(Config[0]);							//

	ScreenCalc Scherm(105,					//init de kleur bereken functies
						Cap.pBits,			//De PixelData
						Cap.return_hres(),	//De Hori Resolutie 
						Cap.return_vres(),	//De Verti Resolutie
						Config[1],					//Hoeveel procent die moet nemen aan de bovenkant/onderkant
						Config[2],					//Hoeveel procent die aan de zijkant moet nemen
						Config[3],				//Leds Boven
						Config[4],					//Leds Onder
						Config[5],					//Leds Links
						Config[6],
						Config[7]);					//Leds Rechts
	
	Scherm.Bereken_Grid();					//stel de hoeveelheid leds in die worden gebruikt en bereken Grid Grootte
	
	//Het programma moet eerst 0xff binnen krijgen als dat het geval is dan mag die beginnen met het oversturen
	//van de hoeveelheid leds
	//Als die hoeveelheden overeenkomen mag die beginnen met het zenden van led data
	Serial* SP = new Serial("\\\\.\\COM5");

	if (SP->IsConnected())
		std::cout << "Connected with COM5" << std::endl;

	char Rx_buffer[100] = "";	//Dit is de Rx_buffer deze moet een char zijn

	//je kan hier op escape drukken om het programma af ste sluiten hier
	std::cout << "Waiting for Arduino. Press ESC to quit" << std::endl;


	while(Rx_buffer[0] != '0') //blijf hier hangen tot de arduino klaar is
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
	UINT8 Tx_buffer[600*3];

	ZeroMemory(Tx_buffer, 600 * 3);
	Tx_buffer[0] = Scherm.geefLeds() >> 8 & 0x00FF;
	Tx_buffer[1] = Scherm.geefLeds() & 0x00FF;

	SP->WriteData((char*)Tx_buffer, 2);

	Sleep(1000);	//Wacht 1 seconde op de arduino

	std::cout << "Press ESC to quit capturing" << std::endl;

	UINT8 *pointer;						//Pointer voor de led bitstream
	pointer = Scherm.GeefLedPointer();	//Koppel de led bitstream aan de pointer
	while (exit == false)
	{
		clock_t begin = clock();

		if (GetAsyncKeyState(VK_ESCAPE))						//Als escape is ingedrukt zet exit true
		{
			exit = true;
		}
		Cap.capture();					//Maak screenshot en sla die op

		Scherm.Bereken();				//Bereken alle led kleuren

		SP->WriteData((char*)pointer, Scherm.geefLeds() * 3);	//Stuur alle data weg


		SP->ReadData(Rx_buffer, 10);
		while (Rx_buffer[0] != '1')		//Wacht tot arduino weer klaar is
		{
			SP->ReadData(Rx_buffer, 100);
		}
		Rx_buffer[0] = '0';
	}

	myinfile.close();
	return 0;
}

void CreateConfig(std::ofstream &file, Direct3DCap &cap)
{
	file.clear();
	//onderstaande code vraagt om scherm
	int i = 1;
	std::cout << "Kies een van de bovenstaande schermen" << std::endl;
	scanf("%d", &i);
	while (i >= cap.return_adapterCounnt() || i < 0)
	{
		std::cout << "Keuze: " << i << " is ongeldig \nKies een van de bovenstaande schermen" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	std::cout << "Hoeveel procent vanaf de zijkant (0-100)" << std::endl;
	scanf("%d", &i);
	while (i < 1 || i > 100)
	{
		std::cout << "Keuze: " << i << " is ongeldig \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	std::cout << "Hoeveel procent vanaf de bovenkant (0-100)" << std::endl;
	scanf("%d", &i);
	while (i < 1 || i > 100)
	{
		std::cout << "Keuze: " << i << " is ongeldig \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	std::cout << "Hoeveel Leds zitter er boven" << std::endl;
	scanf("%d", &i);
	while (i < 1)
	{
		std::cout << "Keuze: " << i << " is ongeldig \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	std::cout << "Hoeveel Leds zitter er onder" << std::endl;
	scanf("%d", &i);
	while (i < 1)
	{
		std::cout << "Keuze: " << i << " is ongeldig \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	std::cout << "Hoeveel Leds zitter er links" << std::endl;
	scanf("%d", &i);
	while (i < 1)
	{
		std::cout << "Keuze: " << i << " is ongeldig \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;

	i = 0;
	std::cout << "Hoeveel Leds zitter er rechts" << std::endl;
	scanf("%d", &i);
	while (i < 1)
	{
		std::cout << "Keuze: " << i << " is ongeldig \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;
	
	i = 0;
	std::cout << "Wat is de minimum zwartwaarde (0- 60)" << std::endl;
	scanf("%d", &i);
	while (i < 1 || i > 60)
	{
		std::cout << "Keuze: " << i << " is ongeldig \n" << std::endl;
		scanf("%d", &i);
	}
	file << i << std::endl;
}