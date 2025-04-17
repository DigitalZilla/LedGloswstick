#include <Arduino.h>
#include <FastLED.h>
#include <AS5600.h>
#include <WiFi.h>

//#include "Drip.h"

#ifdef BOARD_ESP8266
	#define DATA_PIN 2 // D4
#endif

#ifdef BOARD_ESP32_C3
	#define DATA_PIN 4 // D2
#endif

//#define USE_SERIAL

#ifdef USE_SERIAL
	#define PRINT(X) Serial.print(X)
	#define PRINTLN(X) Serial.println(X)
#else
	#define PRINT(X)
	#define PRINTLN(X)
#endif

#define NUM_LEDS 200
//#define NUM_LEDS 400

CRGBArray<NUM_LEDS> Leds;
//AS5600 as5600;

uint16_t Anim_Rainbow;
uint16_t Anim_RainbowShift;
uint32_t Anim_Noise;



void ShutdownWireless()
{
	#ifdef BOARD_ESP8266
	
	#endif

	#ifdef BOARD_ESP32_C3
		btStop();
		WiFi.disconnect();
		WiFi.mode(WIFI_OFF);
	#endif
}

void setup()
{
	ShutdownWireless();

	#ifdef USE_SERIAL
		//while(!Serial);
		Serial.begin(115200);
		Serial.println("-- SETUP --");
	#endif

	// Wire.begin();
	// as5600.begin(4);  //  set direction pin.
	// as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
	// as5600.setOutputMode(AS5600_OUTMODE_ANALOG_100);

	FastLED.addLeds<WS2812, DATA_PIN, GRB>(Leds, NUM_LEDS);  // GRB ordering is assumed
	//FastLED.setBrightness(100);
}


void Ease()
{
	static uint16_t pos = 0;
	static uint8_t led = 0;

	led = lerp8by8(0, NUM_LEDS - 1, ease8InOutQuad(pos >> 8));
	pos += 250;
	
	fadeToBlackBy(Leds, NUM_LEDS, 8);
	Leds[led] = CRGB::Teal;
}

void EaseIn()
{
	uint8_t half = NUM_LEDS / 2;

	for (uint8_t i = 0; i < half; i++)
	{
		Leds[i] = CRGB(map(i, 0, half, 0, 255), 0, 0);
	}
	
	for (uint8_t i = half; i < NUM_LEDS; i++)
	{
		uint8_t val = map(i, 0, half, 0, 255);
		val = scale8(val, val);
		val = scale8(val, val);
		val = scale8(val, val);
		
		Leds[i] = CRGB(val, 0, 0);
	}
	
}

uint16_t scale16Expo(uint16_t val, uint8_t exp)
{
	for (uint8_t i = 0; i < exp; i++)
	{
		val = scale16(val, val);
	}

	return val;
}

void Noise(uint16_t x)
{
	uint16_t dist = x;

	for (uint16_t i = 0; i < NUM_LEDS; i++)
	{
		uint16_t val16 = snoise16((i * dist), Anim_Noise);

		val16 = scale16Expo(val16, 3);

		uint8_t val8 = val16 >> 8;
		Leds[i] = CHSV(0, 255, val8);
	}
}

uint16_t MAX_16 = ((uint16_t)0) - 1;

void FadeByNoise(uint16_t dist, uint8_t exp, uint16_t clip)
{
	for (uint16_t i = 0; i < NUM_LEDS; i++)
	{
		uint16_t val16 = snoise16((i * dist), Anim_Noise);

		val16 = scale16Expo(val16, exp);
		val16 = MAX_16 - val16;
		if (val16 > clip)
		{
			val16 = clip;
		}

		uint8_t val8 = val16 >> 8;
		Leds[i].fadeToBlackBy(val8);
	}
}

#define VARIABLE_LENGTH_ARRAY(TYPE, NAME, SIZE) TYPE NAME[SIZE]

void fill_noise16_Progressive(CRGB *leds, int num_leds,
							  uint8_t octaves, uint16_t x, int scale,
							  uint8_t hue_octaves, uint16_t hue_x, int hue_scale,
							  uint16_t time, uint8_t hue_shift, uint16_t hue_shift_width)
{

	if (num_leds <= 0)
		return;

	for (int j = 0; j < num_leds; j += 255)
	{
		const int LedsRemaining = num_leds - j;
		const int LedsPer = LedsRemaining > 255 ? 255 : LedsRemaining; // limit to 255 max
		if (LedsPer <= 0)
			continue;
		VARIABLE_LENGTH_ARRAY(uint8_t, V, LedsPer);
		VARIABLE_LENGTH_ARRAY(uint8_t, H, LedsPer);

		memset(V, 0, LedsPer);
		memset(H, 0, LedsPer);

		fill_raw_noise16into8(V, LedsPer, octaves, x, scale, time);
		fill_raw_noise8(H, LedsPer, hue_octaves, hue_x, hue_scale, time);

		for (int i = 0; i < LedsPer; ++i)
		{
			uint8_t rollingShift = hue_shift_width * (i + j) >> 8;
			leds[i + j] = CHSV(H[i] + hue_shift + rollingShift, 255, V[i]);
		}
	}
}

void Rainbow(uint8_t width, uint8_t shift = 0, uint16_t shiftWidth = 0)
{
	fill_noise16_Progressive(Leds, NUM_LEDS,
		10,		// octaves (10):			the number of octaves to use for value (brightness) noise 
		0,		// x:						x-axis coordinate on noise map for value (brightness) noise 
		0,		// scale:					the scale (distance) between x points when filling in value (brightness) noise
		1,		// hue_octaves (1-3):		 number of octaves to use for color hue noise
		0,		// hue_x:					x-axis coordinate on noise map for color hue noise
		width,	// hue_scale (0 - 100):		the scale (distance) between x points when filling in color hue noise
		Anim_Rainbow,
		shift,
		shiftWidth
	);
}

// #define NUM_DRIPS 4
// Drip Drips[NUM_DRIPS];

void loop()
{
	//uint16_t angle = as5600.readAngle();
	uint16_t angle = 0;
	uint16_t x = map(angle, 0, 4095, 50000, MAX_16);

	//Noise(x);
	//Ease();
	//EaseIn();

	Rainbow(30, Anim_RainbowShift >> 8, 200);
	//Rainbow(30, x);
	FadeByNoise(1500, 3, 60500);

	// for (uint8_t i = 0; i < NUM_DRIPS; i++)
	// {
	// 	Drips[i].Update();
	// 	Drips[i].Render(Leds);
	// }
	// Leds.fadeToBlackBy(10);

	EVERY_N_MILLISECONDS(500)
	{
		PRINT(angle);
		PRINT("\t");
		PRINT(x);
		PRINTLN();
	}

	EVERY_N_MILLISECONDS(10)
	{
		Anim_Rainbow += 1;
		Anim_RainbowShift += 40;
		Anim_Noise += 12;
	}

	FastLED.delay(10);
}