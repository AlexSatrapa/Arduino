#include "pins.h"
#include <DHT.h>

#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

void setup () {
	Serial1.begin(9600);
	Serial1.write(0x76);
	dht.begin();
	}

void loop () {
	char tempString[6];
	float h = dht.readHumidity();
	float t = dht.readTemperature();
	if (isnan(h) || isnan(t)) {
		Serial.println("Failed to read from sensor!");
		return;
		}
	int tint = t;
	int tdec = (t - tint) * 10;
	sprintf(tempString, "%2d", tint);
	// Move cursor to start of line
	Serial1.write(0x79);
	Serial1.write(0);
	Serial1.print(tempString);
	// Decimal point
	Serial1.write(0x77);
	Serial1.write(0x02);
	Serial1.write(tdec);
	// be fancy and write the º symbol
	Serial1.write(0x7E);
	Serial1.write(0b01100011);
	delay(2000);
	int hint = h;
	int hdec = (h - hint) * 10;
	sprintf(tempString, "%2d", hint);
	Serial1.write(0x79);
	Serial1.write(0);
	Serial1.print(tempString);
	Serial1.write(0x77);
	Serial1.write(0x02);
	Serial1.write(hdec);
	Serial1.write('h');
	delay(2000);
	}
