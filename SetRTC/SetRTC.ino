#include "pins.h"
#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 RTC;

void setup() {
	// put your setup code here, to run once:
	Serial.begin(57600);       // Use a higher speed to spend less time awake!
	Serial.println("Setting RTC date and time.");
	Wire.begin();
	RTC.begin();
	// following line sets the RTC to the date & time this sketch was compiled
	//RTC.adjust(DateTime(__DATE__, __TIME__));
	//RTC.adjust(DateTime("Feb 01 2015", "00:26:30"));
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	// note that the parser is very simple, you must use two-digit days and four digit year
}

void print_datestamp() {
	DateTime now;
	now = RTC.now();
	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(' ');
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println('.');
	}

void loop() {
	// put your main code here, to run repeatedly:
	print_datestamp();
	delay(1000);
}
