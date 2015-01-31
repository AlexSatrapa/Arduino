#include "pins.h"
#include <Wire.h>
#include "DS3231.h"
#include <avr/power.h>
#include <avr/sleep.h>

char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

DS3231 RTC;

volatile uint8_t interrupted = 0;

void handle_alarm() {
	detachInterrupt(0);
	interrupted = 1;
	}

void arm_alarm() {
	RTC.clearINTStatus();
	interrupted = 0;
	RTC.enableInterrupts(EveryHour);
	attachInterrupt(0, handle_alarm, LOW);
	}

void setup_pins() {
	// Receive alarm interrupts
	pinMode(DS3231_ALARM, INPUT);
	digitalWrite(DS3231_ALARM, HIGH);

	// Turn off the XBee and microSD card to save power
	pinMode(POWER_BEE, OUTPUT);
	digitalWrite(POWER_BEE, LOW);
	pinMode(POWER_TF, OUTPUT);
	digitalWrite(POWER_TF, LOW);
	}

void system_sleep() {
	power_all_disable();
	digitalWrite(POWER_BEE, LOW);
	digitalWrite(POWER_TF, LOW);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();
	power_all_enable();
	}

// Brute force: turn the XBee on, wait a short while for it to boot up.
void zigbee_enable() {
	digitalWrite(POWER_BEE, HIGH);
	delay(7000);
	}

// Bruge force: don't check status, just cut it off.
void zigbee_disable() {
	Serial.flush();
	delay(10000); // allow time for programming
	digitalWrite(POWER_BEE, LOW);
	}

void setup() {
	Serial.begin(57600);
	setup_pins();
	Wire.begin();
	RTC.begin();
	arm_alarm();
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
	system_sleep();
	digitalWrite(LED, HIGH);
	zigbee_enable();
	print_timestamp();
	zigbee_disable();
	arm_alarm();
	digitalWrite(LED, LOW);
	}
