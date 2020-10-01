#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Turn on debug statements to the serial output
#define DEBUG 0

unsigned long currentMillis = millis();

#if DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTX(x) Serial.println(x, HEX)
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTX(x)
#endif

//Define the pins for sensors

// Define the pins for LED matrix
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 25;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 1000; // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
#define	BUF_SIZE	128
char currentMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "Initialing..." };
bool newMessageAvailable = true;

void displayMessage(String str)
{
	static char* cp = newMessage;
	
	for (int count = 0; str[count] != '\0'; count++)//我日你妈foreach 编译报错
	{
		*cp = str[count];
		if ((*cp == '\n') || (cp - newMessage >= BUF_SIZE - 2)) // end of message character or full buffer
		{
			*cp = '\0'; // end the string
			// restart the index for next filling spree and flag we have a message waiting
			cp = newMessage;
			newMessageAvailable = true;
		}
		else
		{
			// move char pointer to next position
			cp++;
		}
	}
}

void updateScreen()
{
	if (P.displayAnimate())
	{
		if (newMessageAvailable)
		{
			strcpy(currentMessage, newMessage);
			newMessageAvailable = false;
		}
		P.displayReset();
	}
}

void setup() {
  // put your setup code here, to run once:
  P.begin();
  P.displayText(currentMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
}

void loop() {
  // put your main code here, to run repeatedly:
  updateScreen();
}
