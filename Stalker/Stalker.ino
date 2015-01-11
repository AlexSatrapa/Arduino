#include "pins.h"
#include <Wire.h>
#include "DS3231.h"

char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

DS3231 RTC;

volatile uint8_t interrupted = 0;

void handle_alarm() {
	interrupted = 1;
	}

void arm_alarm() {
	attachInterrupt(0, handle_alarm, FALLING);
	interrupted = 0;
	RTC.clearINTStatus();
	}

void setup_pins() {
	pinMode(DS3231_ALARM, INPUT);
	digitalWrite(DS3231_ALARM, HIGH);
	pinMode(POWER_BEE, OUTPUT);
	digitalWrite(POWER_BEE, HIGH);
	pinMode(POWER_TF, OUTPUT);
	digitalWrite(POWER_TF, LOW);
	}

void setup() {
	Serial.begin(57600);
	setup_pins();
	Wire.begin();
	RTC.begin();
	arm_alarm();
	RTC.enableInterrupts(EveryMinute);
	Serial.println("RESET");
	}

void print_timestamp () {
		DateTime now = RTC.now(); //get the current date-time
		Serial.print(now.year(), DEC);
		Serial.print('/');
		Serial.print(now.month(), DEC);
		Serial.print('/');
		Serial.print(now.date(), DEC);
		Serial.print(' ');
		Serial.print(now.hour(), DEC);
		Serial.print(':');
		Serial.print(now.minute(), DEC);
		Serial.print(':');
		Serial.print(now.second(), DEC);
		Serial.print(' ');
		Serial.print(weekDay[now.dayOfWeek()]);
		Serial.println();
	}

void print_temperature () {
		RTC.convertTemperature();             //convert current temperature into registers
		Serial.print(RTC.getTemperature()); //read registers and display the temperature
		Serial.println("deg C");
	}

void loop () {
	if (interrupted == 1) {
		Serial.println("INTERRUPTED");
		interrupted = 0;
		RTC.clearINTStatus();
		}
	print_timestamp();
	for (int i=0; i < 20; i++) {
		digitalWrite(LED, 1);
		delay(250);
		digitalWrite(LED, 0);
		delay(250);
		}
	}
