// include the basic windows header files and the Direct3D header file
#include <iostream>
#include <fstream>
#include <ctime>
#include <windows.h>

#include "Direct3DCap.h"
#include "ScreenCalc.h"
#include "SerialClass.h"




int main()
{
	bool exit = false;						//dit is voor later een escape variable

	Direct3DCap Cap;						//init directx
	Cap.init(1);							//

	ScreenCalc Scherm(105,					//init de kleur bereken functies
						Cap.pBits,			//De PixelData
						Cap.return_hres(),	//De Hori Resolutie 
						Cap.return_vres(),	//De Verti Resolutie
						20,					//Hoeveel procent die moet nemen aan de bovenkant/onderkant
						20,					//Hoeveel procent die aan de zijkant moet nemen
						60);				//Leds per Meter
	
	Scherm.Bereken_Grid();					//stel de hoeveelheid leds in die worden gebruikt en bereken Grid Grootte
	
	//Het programma moet eerst 0xff binnen krijgen als dat het geval is dan mag die beginnen met het oversturen
	//van de hoeveelheid leds
	//Als die hoeveelheden overeenkomen mag die beginnen met het zenden van led data
	Serial* SP = new Serial("\\\\.\\COM14");

	if (SP->IsConnected())
		std::cout << "Connected with COM14" << std::endl;

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

	Sleep(100);	//Wacht 100 miliseconden op antwoord

	std::cout << "Press ESC to quit capturing" << std::endl;

	UINT8 *pointer;						//Pointer voor de led bitstream
	pointer = Scherm.GeefLedPointer();	//Koppel de led bitstream aan de pointer
	while (exit == false)
	{
		SP->WriteData((char*)pointer, Scherm.geefLeds() * 3);	//Stuur alle data weg
		clock_t begin = clock();

		if (GetAsyncKeyState(VK_ESCAPE))						//Als escape is ingedrukt zet exit true
		{
			exit = true;
		}
		Cap.capture();					//Maak screenshot en sla die op

		Scherm.Bereken();				//Bereken alle led kleuren
	
		

		while (Rx_buffer[0] != '1')		//Wacht tot arduino weer klaar is
		{
			SP->ReadData(Rx_buffer, 100);
		}
		Rx_buffer[0] = '0';

		clock_t end = clock();
		std::cout << "FPS: " << CLOCKS_PER_SEC / double(end - begin) << std::endl;	//Print de FPS uit
	}
	pointer = NULL;	//Zorg dat Pointer nergens meer naar wijst
	return 0;
}
