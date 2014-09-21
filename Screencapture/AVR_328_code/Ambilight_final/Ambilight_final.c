/*
 * ambilight.c
 * Deze software werkt
 * Created: 9-9-2014 18:49:06
 *  Author: Job-Game
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "includes/light_ws2812.h"

#define DEBUG 0			//#if DEBUG. Als je dat gebruikt kan je hier debug mee aan en uit zetten

uint16_t  Led_count = 0;
uint16_t i,j;
uint8_t LedArray[1800] = {0};

void CycleLeds();

int main(void)
{
	//Schakel sowieso alle leds uit	
	for(i=0;i<3*600;i++)
	{
		LedArray[i] = 0;
	}
	ws2812_sendarray(LedArray,1800);
	
	
	//UCSR0A = (1<<U2X0);
	UCSR0B = ((1 << RXEN0) | (1 << TXEN0) | (1<<RXCIE0));   // Zet de Serial communicatie aan. RXCIE0 is voor interrupt | (1<<RXCIE0)
	UCSR0C = ((1<<UCSZ01)|(3<<UCSZ00)); // Gebruik 8-bit character sizes - 1 bits stop
	UBRR0H = 0; // Laad de eerste 8 bits naar het UBBRH register
	UBRR0L = 0x01; // Laad de laatste 8 bits naar het UBBRL register
	
	_delay_ms(2000);		//geed de computer tijd om alles te initen
	uint8_t firstbyte = 0;
	
	while((Led_count & 0x00FF) < 1)
	{
		_delay_ms(100);
		if(UCSR0A & (1<<UDRE0) && firstbyte == 0);	//als de data buffer vrij is stuur 0x30
		{
			UDR0 = 0x30;
		}
		
		
		if (UCSR0A & (1 << RXC0))	//Check voor antwoord
		{
			
			if(firstbyte == 1)
				Led_count |= UDR0 & 0x00FF;
			else
			{
				Led_count = UDR0<<8;
				firstbyte = 1;
			}
				
			
		}
		
	}
	
	
	#if DEBUG
		//echo de led data terug voor bevestiging
		while(!( UCSR0A & (1<<UDRE0)));
		UDR0 = Led_count>>8;
		while(!( UCSR0A & (1<<UDRE0)));
		UDR0 = Led_count & 0x00FF;
	#endif
	
	
	
	if (Led_count > 1)
	{
		sei();
		while(1)
		{
			//Doe niks hier
			//Wacht op led data 
			//alles word afgehandeld in interrupt
					
		}
	}
	else
	{
		while(1)
		{
			//Doe anders dit patroon
		}
	}
	
	
    
}

ISR(USART_RX_vect)
{
	static uint16_t data_count = 0;

	
	LedArray[data_count] = UDR0;
	
	if (data_count >= ((Led_count*3)-1))
	{
		ws2812_sendarray(LedArray,Led_count*3);	
		data_count = 0;
		UDR0 = 0x31;
		
			
	}
	else
	{
		
		data_count++;
	}
	
	/*
	uint8_t rest;
	
	rest = data_count % 3;
	switch (rest)
	{
		case 0:
			led[data_count/3].r = UDR0;
			#if DEBUG 
			UDR0 = led[data_count/3].r; 
			#endif
		break;
	
		case 1:
			led[data_count/3].g = UDR0;
			#if DEBUG
			UDR0 = led[data_count/3].g;
			#endif
		break;
	
		case 2:
			led[data_count/3].b = UDR0;
			#if DEBUG
			UDR0 = led[data_count/3].b;
			#endif
		break;
	}
	
	
	if (data_count >= ((Led_count*3)-1))
	{
		ws2812_setleds(led,Led_count);
		data_count = 0;
		UDR0 = 0x31;
	}
	else
	{
		data_count++;
	}*/
	
}
