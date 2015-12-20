#include "pins.h"
#include <Sleep.h>
#include <SPI.h>
#include <DSRTC.h>
#include <ds3234.h>

const byte BUFF_MAX = 32;
DS3234 RTC = DS3234(SS);
dsrtc_calendar_t alarmSetting;
char buff[BUFF_MAX];

ISR (PCINT1_vect) {
	GIMSK &= ~B00010000; // Disable PCI group 1
	}

void enableInterrupt() {
	PCMSK1 |= B00000100; // Enable PCINT10, Arduino pin 2
	GIMSK |= B00100000; // Enable PCI group 1
	}

inline void printDatestamp() {
	dsrtc_calendar_t t;
	RTC.read(t);

	// Current time
	snprintf(buff, BUFF_MAX, "%d/%02d/%02dT%02d:%02d:%02d", t.Year, t.Month, t.Day, t.Hour, t.Minute, t.Second);
	Serial.print(buff);
	}

void setup() {
	// put your setup code here, to run once:
	pinMode(ALARM_PIN, INPUT_PULLUP);
	pinMode(XBEE_ENABLE, OUTPUT);
	digitalWrite(XBEE_ENABLE, HIGH);
	Serial.begin(9600);
	Serial.println("Starting.");

	alarmSetting.Year = 15; // type only supports 255, RTC only supports 0-99
	alarmSetting.Month = 8;
	alarmSetting.Wday = 1;
	alarmSetting.Day = 30;
	alarmSetting.Hour = 01;
	alarmSetting.Minute = 00;
	alarmSetting.Second = 00;
	RTC.writeAlarm(2, alarmModePerMinute, alarmSetting);
	RTC.setSQIMode(sqiModeAlarm2);
}

void loop() {
	// put your main code here, to run repeatedly:
	RTC.clearAlarmFlag(DS323X_ALARM_BOTH);
	digitalWrite(XBEE_ENABLE, HIGH);
	printDatestamp();
	Serial.println(" Ping!");
	Serial.flush();
	delay(1);
	enableInterrupt();
	digitalWrite(XBEE_ENABLE, LOW);
	powerDown();	
}
