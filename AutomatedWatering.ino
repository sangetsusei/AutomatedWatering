#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


// Turn on debug statements to the serial output
#define DEBUG 0

// will store last update time
unsigned long previousMillis_Ssr = 0;
unsigned long previousMillis_CheckWater = 0;
unsigned long previousMillis_Pump = 0;
bool setpreviousMillis_CheckWaterFlag = true;

#if DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTX(x) Serial.println(x, HEX)
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTX(x)
#endif

// Sensors
const long sensorsUpdateInterval = 3000; //ms
const long waterLevelCheckOffsetInterval = 100000; //ms
const long wateringTime = 5000; //ms

#define DHTPIN 3     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT11     // DHT 11 type sensor
DHT_Unified dht(DHTPIN, DHTTYPE);

#define WSPIN A4

double temperature =  0;
double humidity = 0;
int waterLevel = 100;

static char tempratureOutstr[16];
static char humidityOutstr[16];
static char waterLevelOutstr[16];

// LED matrix
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

//Relays
#define RELAY_1 8
#define RELAY_2 9 
int Relay1State = HIGH;
int Relay2State = LOW;


// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 30;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 0; // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
#define	BUF_SIZE	128
char currentMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "Initialing..." };
bool newMessageAvailable = true;

void displayMessage(String str)
{
	static char* cp = newMessage;
	
	for (int count = 0; str[count] != '\0'; count++)// Why can't I use Foreach???
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

void updateRealys()
{
	digitalWrite(RELAY_1, Relay1State);
	digitalWrite(RELAY_2, Relay2State);
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

void updateSensorValue()
{
	// Get temperature event
	sensors_event_t event;
	dht.temperature().getEvent(&event);
	if (isnan(event.temperature)) 
	{
    	PRINTS("Error reading temperature!");
	}
	else {
    	temperature = event.temperature;
	}
  	// Get humidity event
		dht.humidity().getEvent(&event);
	if (isnan(event.relative_humidity)) 
	{
		PRINTS("Error reading humidity!");
	}
	else {
    	humidity = event.relative_humidity;
	}
	Serial.println(analogRead(WSPIN));
	waterLevel = map(analogRead(WSPIN), 400, 960 , 0, 100);
}

void setup() {
  // put your setup code here, to run once:
	Serial.begin(9600);
	dht.begin();
	P.begin();
	P.displayText(currentMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
	pinMode(RELAY_1, OUTPUT);
	digitalWrite(RELAY_1, HIGH); //Turn off the realy first
}

void loop() 
{
  // put your main code here, to run repeatedly:
	static uint8_t	displayStatus = 0;
	unsigned long currentMillis = millis();

	updateScreen();
	updateRealys();

	if (currentMillis - previousMillis_Ssr >= sensorsUpdateInterval) {
		// save the last time you update the Sensors value
		previousMillis_Ssr = currentMillis;
		updateSensorValue();
		Serial.println("Updated sensors");
	}

	if (waterLevel <= 30)
	{
		if (setpreviousMillis_CheckWaterFlag)
		{
			Serial.println("water level low");
			previousMillis_CheckWater = currentMillis;
			setpreviousMillis_CheckWaterFlag = false;
		}
		
		if ((currentMillis - previousMillis_CheckWater >= waterLevelCheckOffsetInterval)&&(Relay1State == HIGH))
		{
			Serial.println("Start pump");
			Relay1State = LOW;
			previousMillis_Pump = currentMillis;
		}
	}

	if ((currentMillis - previousMillis_Pump >= wateringTime)&&(Relay1State == LOW))
	{
		Serial.println("Done watering");
		Relay1State = HIGH;
		setpreviousMillis_CheckWaterFlag = true;
	}
	
	


	if (temperature != 0)
	{
		switch (displayStatus)
		{
		case 0:
			displayMessage("Air Temperature = ");
			dtostrf(temperature, 4, 2, tempratureOutstr);
			displayMessage(tempratureOutstr);
			//Serial.println(temperature);
			displayMessage(" 'C      ");
			displayStatus++;
			//Serial.println("x");
			break;
		case 1:
			displayMessage("Air Humidity = ");
			dtostrf(humidity, 2, 0, humidityOutstr);
			displayMessage(humidityOutstr);
			displayMessage(" %            ");
			displayStatus++;
			//Serial.println("x");
			break;
		default:
			displayMessage("Water Level = ");
			dtostrf(waterLevel, 3, 0, waterLevelOutstr);
			displayMessage(waterLevelOutstr);
			displayMessage(" %              ");
			displayStatus = 0;
			break;
		}

	}
	
	//P.displayReset(0); 
}
