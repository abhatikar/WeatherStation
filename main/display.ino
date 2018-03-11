// mapping suggestion from Waveshare 2.9inch e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

#include <GxEPD.h>
#include <GxGDEW027C44/GxGDEW027C44.cpp>    // 2.7" b/w/r
//#include <GxGDEW027W3/GxGDEW027W3.cpp>      // 2.7" b/w
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

// FreeFonts from Adafruit_GFX
//#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>

//#include GxEPD_BitmapExamples

#include "imagedata.h"

#if defined(ESP8266)

// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, SS, 0, 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 2, uint8_t busy = 4);
GxEPD_Class display(io); // default selection of D4(=2), D2(=4)
#endif

//#define DEMO_DELAY 3*60 // seconds
//#define DEMO_DELAY 1*60 // seconds
#define DEMO_DELAY 30

void display_setup(void)
{
  Serial.println();
  Serial.println("setup display start");
  display.init();
  Serial.println("setup display done");
}

void display_logic(String weatherData)
{
//  showPartialUpdate();
	showFontCallback(weatherData);
  //delay(DEMO_DELAY * 1000);
}

#if !defined(__AVR)

void showFontCallback(String weatherData)
{

	const char* date = "20 Dec 2018";
	const unsigned char *bitmap;

	DynamicJsonBuffer  jsonBuffer(2000);

	JsonObject& root = jsonBuffer.parseObject(weatherData);

	// Test if parsing succeeds.
	if (!root.success()) {
		Serial.println("parseObject() failed");
		return;
	}

	JsonArray& list = root["list"];
	JsonObject& now = list[0];
	JsonObject& later = list[1];

	String city = root["city"]["name"];
	float tempNow = now["main"]["temp"];
	float humidityNow = now["main"]["humidity"];
	String weatherNow = now["weather"][0]["description"];

	Serial.println(city);
	Serial.println(tempNow);
	Serial.println(humidityNow);
	Serial.println(weatherNow);

	const GFXfont* f = &FreeSansBoldOblique9pt7b;

	uint16_t box_x = 10;
	uint16_t box_y = 15;
	uint16_t box_w = 150;
	uint16_t box_h = 22;
	uint16_t cursor_y = box_y + box_h - 6;

	display.fillScreen(GxEPD_WHITE);
	display.setTextColor(GxEPD_BLACK);
	f = &FreeSansBoldOblique9pt7b;
	display.setFont(f);
	display.setRotation(3);

	display.fillRect(0, 0, box_w, box_h, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setCursor(2, 13);
	display.print("Weather Station   ");

	f = &FreeSansOblique9pt7b;
	display.setFont(f);
	display.setTextColor(GxEPD_BLACK);
	display.setCursor(150, 15);
	display.println(date);

	display.println();
	f = &FreeSansBoldOblique9pt7b;
	display.setFont(f);

	display.setTextColor(GxEPD_BLACK);
	display.setCursor(0, 38);
	display.println(city);

	//display.println(sunrise);
	//display.println(sunset);
	Serial.print("Temp:");
	Serial.print(tempNow);
	Serial.println("*C");
	Serial.print("Humidity:");
	Serial.print(humidityNow);
	Serial.println("%");
	Serial.println(weatherNow);

	display.println(weatherNow);
	display.print("Temp:");
	display.print(tempNow);
	display.println("*C");
	display.print("Humidity:");
	display.print(humidityNow);
	display.println("%");
	display.setCursor(135, 125);
	display.println("RoomTemp");
	display.setCursor(135, 145);
	display.println(tempNow);
	display.setCursor(135, 165);
	display.println(humidityNow);

	if(weatherNow == "clear sky")
		bitmap = bm_moon;

	if((weatherNow == "few clouds") || (weatherNow == "scattered clouds") || (weatherNow == "broken clouds"))
		bitmap = bm_cloud;

	if (weatherNow.indexOf("rain") > 0)
		bitmap = bm_rain;

	if(weatherNow.indexOf("shower rain") > 0)
		bitmap = bm_lightrain;

	if(weatherNow.indexOf("thunderstorm") > 0)
		bitmap = bm_thunder;

	if(weatherNow.indexOf("snow") > 0)
		bitmap = bm_snow;

	if(weatherNow == "mist")
		bitmap = bm_clear;

	display.drawBitmap(bitmap, 150, 30, 110, 80, GxEPD_WHITE);
	display.update();


#if 0
	//  display.drawBitmap(bm_wind, 150, 30, 110, 80, GxEPD_WHITE);
	//  display.update();
	//
	//  //display.drawBitmap(bm_thunder, 150, 30, 110, 80, GxEPD_WHITE);
	//  //display.update();
	//
	//  display.drawBitmap(bm_sunny, 150, 30, 110, 80, GxEPD_WHITE);
	//  display.update();
	//
	//  //display.drawBitmap(bm_snow, 150, 30, 110, 80, GxEPD_WHITE);
	//  //display.update();
	//
	//  //display.drawBitmap(bm_rain, 150, 30, 110, 80, GxEPD_WHITE);
	//  //display.update();
	//
	//  display.drawBitmap(bm_pl_night, 150, 30, 110, 80, GxEPD_WHITE);
	//  display.update();
	//  
	//  display.drawBitmap(bm_pl_day, 150, 30, 110, 80, GxEPD_WHITE);
	//  display.update();
	//
	//  display.drawBitmap(bm_moon, 150, 30, 110, 80, GxEPD_WHITE);
	//  display.update();
	//
	//  //display.drawBitmap(bm_lightrain, 150, 30, 110, 80, GxEPD_WHITE);
	//  //display.update();
#endif
	//display.drawBitmap(bm_cloud, 150, 30, 110, 80, GxEPD_WHITE);
	//display.update();

	//display.drawBitmap(bm_clear, 150, 30, 110, 80, GxEPD_WHITE);
	//display.update();


	//display.drawBitmap(bm_snow, 150, 30, 110, 80, GxEPD_BLACK);
	//display.update();
	//display.drawBitmap(bm_cloud, 150, 30, 110, 80, GxEPD_BLACK);
	//display.update();
	//display.drawBitmap(bm_sun, 150, 30, 110, 80, GxEPD_BLACK);
	//display.update();
	//delay(2000);
	// partial update to full screen to preset for partial update of box window
	// (this avoids strange background effects)
	//display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
}

#endif
