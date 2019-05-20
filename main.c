//Displaying "HELLO" on LCD for Dragon12+ Trainer Board 
//with HCS12 Serial Monitor Program installed. This code is for CodeWarrior IDE
//Modified from Mazidi's book with contribution from Travis Chandler
//On Dragon12+ LCD data pins of D7-D4 are connected to Pk5-Pk2, En=Pk1,and RS=Pk0,


#include <hidef.h>      /* common defines and macros */
#include "mc9s12dg256.h"      /* derivative-specific definitions */
#include <stdlib.h>
#include <string.h>



#define LCD_DATA PORTK
#define LCD_CTRL PORTK
#define RS 0x01
#define EN 0x02
#define refLight 100 /* threshold value of the light to start the speaker */
#define refTemp 25 /* threshold value of the temperature to start the LEDs */

void COMWRT4(unsigned char);
void DATWRT4(unsigned char);
void MSDelay(unsigned int);
void printString(char *ptr);
void initLightSensor();
void initTempSensor();
void temp_alarm(int temp);
void light_alarm(int light);
void main(void)
{

	char *msg1 = "Light:";
	char *msg2 = "Temp:";
	char *msgLX = "lx";
	char *clrMsg = "     ";
	int i;
	int temp, light, light_aux;


	//init data conversion registers for sensors

	DDRB = 0xFF; /* port B is set as output */
	DDRJ = 0xFF; /* port J is set as output to control Dragon12+ LEDs */
	PTJ = 0x00; /* allow the LEDs to display data on PORTB pins */
	DDRK = 0xFF; /* port K for LCD is set as output */
	DDRT |= 0x20; /* pin 5 of port T is set as output (speaker) */

	COMWRT4(0x33);   /* reset sequence provided by data sheet */
	MSDelay(1);
	COMWRT4(0x32);   /* reset sequence provided by data sheet */
	MSDelay(1);
	COMWRT4(0x28);   /* Function set to four bit data length */
									 /* 2 line, 5 x 7 dot format */
	MSDelay(1);
	COMWRT4(0x06);  /* entry mode set, increment, no shift */
	MSDelay(1);
	COMWRT4(0x0E);  /* Display set, disp on, cursor on, blink off */
	MSDelay(1);
	COMWRT4(0x01);  /* Clear display */
	MSDelay(1);
	COMWRT4(0x80);  /* set start posistion, home position, scrierea incepe de pe prima pozitie de pe linia 1 */
	MSDelay(1);
	printString(msg1); /* the message "Light" is displayed */
	COMWRT4(0x8D);
	printString(msgLX); /* the unit of measure for luminous intensity "lx" is displayed */
	COMWRT4(0xC0);  /* write on line 2 at position 0 */
	MSDelay(1);
	printString(msg2); /* the message "Temp" is displayed*/
	COMWRT4(0xCB);
	DATWRT4(0xDF);
	COMWRT4(0xCC); /* the unit of measure for temperature, degrees Celsius is displayed (+ next line) */
	DATWRT4('C');


	for (;;) { /* forever loop */
		initLightSensor(); /* initialize the light sensor */
		MSDelay(10);

		while (!(ATD0STAT0 & 0x80)); /* wait to complete a conversion sequence */

		if (ATD0STAT0 & 0x80) { /* if a onversion sequence has completed */
			COMWRT4(0x86);
			printString(clrMsg); /* clear the characters displayed before */
			light = ATD0DR0H * 39.06; /* light conversion from voltage to lx */
			light_aux = light;
			i = 0x8A;
			while (light != 0) {
				COMWRT4(i);
				DATWRT4(light % 10 + '0'); /* display the value as a character, not a integer number (integer to ascii conversion) */
				i--;
				light /= 10;
			} /* display the light on the screen, every character from the light value is displayed on the screen and decrement the position of the character that is going to display */
			light_alarm(light_aux); /* the speaker alarm is activated when the value of the light recieved from the sensor is less than the threshold value */
			ATD0STAT0 |= 0x80; /* reset the sequence complete flag */
		}

		MSDelay(10);

		initTempSensor(); /* initialize the temperature senor */
		MSDelay(10);

		while (!(ATD0STAT0 & 0x80)); /* wait to complete a conversion sequence */

		if (ATD0STAT0 & 0x80) { /* if a onversion sequence has completed */
			COMWRT4(0xC5);
			printString(clrMsg); /* clear the characters displayed before */
			temp = ATD0DR0H * 1.9; /* temperature conversion from voltage to Celsius degrees */
			temp_alarm(temp); /* LEDs are activated when the temperature read from the sensor is higher than the threshold value (active LEDs: 7, 5, 3, 1) */
			i = 0xC7;
			while (temp != 0) {
				COMWRT4(i);
				DATWRT4(temp % 10 + '0'); /* display the value as a character, not a integer number (integer to ascii conversion) */
				i--;
				temp /= 10;
			} /* display the temperature on the screen, every character from the temperature value is displayed on the screen and decrement the position of the character that is going to display */
			ATD0STAT0 |= 0x80; /* reset the sequence complete flag */
		}
		MSDelay(1000);
	}
}

void COMWRT4(unsigned char command)
{
	unsigned char x;

	x = (command & 0xF0) >> 2; /* shift high nibble to center of byte for Pk5-Pk2 */
	LCD_DATA = LCD_DATA & ~0x3C; /* clear bits Pk5-Pk2 */
	LCD_DATA = LCD_DATA | x; /* sends high nibble to PORTK */
	MSDelay(1);
	LCD_CTRL = LCD_CTRL & ~RS; /* set RS to command (RS=0) */
	MSDelay(1);
	LCD_CTRL = LCD_CTRL | EN; /* rais enable */
	MSDelay(5);
	LCD_CTRL = LCD_CTRL & ~EN; /* Drop enable to capture command */
	MSDelay(15); /* wait */
	x = (command & 0x0F) << 2; /* shift low nibble to center of byte for Pk5-Pk2 */
	LCD_DATA = LCD_DATA & ~0x3C; /* clear bits Pk5-Pk2 */
	LCD_DATA = LCD_DATA | x; /* send low nibble to PORTK */
	LCD_CTRL = LCD_CTRL | EN; /* rais enable */
	MSDelay(5);
	LCD_CTRL = LCD_CTRL & ~EN; /* drop enable to capture command */
	MSDelay(15);
}

void DATWRT4(unsigned char data)
{
	unsigned char x;



	x = (data & 0xF0) >> 2;
	LCD_DATA = LCD_DATA & ~0x3C;
	LCD_DATA = LCD_DATA | x;
	MSDelay(1);
	LCD_CTRL = LCD_CTRL | RS;
	MSDelay(1);
	LCD_CTRL = LCD_CTRL | EN;
	MSDelay(1);
	LCD_CTRL = LCD_CTRL & ~EN;
	MSDelay(5);

	x = (data & 0x0F) << 2;
	LCD_DATA = LCD_DATA & ~0x3C;
	LCD_DATA = LCD_DATA | x;
	LCD_CTRL = LCD_CTRL | EN;
	MSDelay(1);
	LCD_CTRL = LCD_CTRL & ~EN;
	MSDelay(15);
}


void printString(char *ptr)
{
	while (*ptr) { /* While character to send */
		DATWRT4(*ptr); /* Write data to LCD */
		MSDelay(1); /* Wait for data to be written */
		ptr++; /* Go to next character */
	}
}

void initLightSensor() {

	ATD0CTL2 = 0x80; /* normal ATD functionality */
	ATD0CTL3 = 0x08; /* 1 conversion/sequence */
	ATD0CTL4 = 0xEB; /* 8 bit resolution, 16 A/D conversion clock periods */
	ATD0CTL5 = 0x24; /* 8-bit / right justified / unsigned - bits 0-7, continuous conversion sequences (scan mode), Analog Input Channel 4 */
}

void initTempSensor() {

	ATD0CTL2 = 0x80; /* normal ATD functionality */
	ATD0CTL3 = 0x08; /* 1 conversion/sequence */
	ATD0CTL4 = 0xEB; /* 8 bit resolution, 16 A/D conversion clock periods */
	ATD0CTL5 = 0x25; /* 8-bit / right justified / unsigned - bits 0-7, continuous conversion sequences (scan mode), Analog Input Channel 5 */

}

void temp_alarm(int temp) {
	if (temp > refTemp) /* if the given temperature is greater than threshold value */
		PORTB = 0xAA; /* turn on the LEDs (7, 5, 3, 1) */
	else
		PORTB = 0x00; /*turn off the LEDs */
}

void light_alarm(int light) {
	int i;
	if (light < refLight) { /* if the given light is less than threshold value */
		for (i = 0; i < 6; i++) { /* repeat 6 times the sound */
			PTT |= 0x20; /* turn on the speaker */
			MSDelay(100); /* wait 100 miliseconds */
			PTT &= 0x00; /* turn off the speaker */
			MSDelay(100); /* wait 100 miliseconds */
		}
	}
	else
		PTT &= 0x00; /* turn off the speaker */
}


void MSDelay(unsigned int itime)
{
	unsigned int i; unsigned int j;
	for (i = 0; i < itime; i++)
		for (j = 0; j < 4000; j++);
}