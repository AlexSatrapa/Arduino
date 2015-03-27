#include "pins.h"
#include <HSCDANN005PGSA5.h>
#include <DS3234.h>
#include <Sleep.h>

const int MIN_RAW = 1638;
const int MAX_RAW = 14746;
const int BUFF_MAX = 80;
const int ERROR = (MAX_RAW - MIN_RAW) / 400; // 0.25%
const int HIGH_CUTOUT = MAX_RAW * 0.9;
dsrtc_calendar_t time_buf;
volatile bool ALARM = false;

HSCDANN005PGSA5 pressure_sensor(PRESSURE);
DS3234 rtc(RTC_SS_PIN, DS323X_INTCN || DS323X_EOSC);

void print_datestamp() {
	char buff[BUFF_MAX];
	dsrtc_calendar_t t;
	rtc.read(t);

	// Current time
	snprintf(buff, BUFF_MAX, "%d/%02d/%02dT%02d:%02d:%02d", t.Year, t.Month, t.Day, t.Hour, t.Minute, t.Second);
	Serial.print(buff);
	}

void print_values() {
	char buff[BUFF_MAX];
	int raw_press = pressure_sensor.pressure_Raw();
	int raw_temp = pressure_sensor.temperature_Raw();
	snprintf(buff, BUFF_MAX, ", %d, %d", raw_press, raw_temp);
	Serial.println(buff);
	}

void printReading() {
	print_datestamp();
	print_values();
	}

/*
How a reading is taken:

1. Start off with a pressure reading.

2. Blow some air into the tube.

3. If the pressure in the tube doesn't change significantly (i.e.: by more than the margin of error of the sensor), consider the tube evacuated, and the air pressure to be at equilibrium with the water pressure.

4. If the pressure does change, go back to step 1.
*/
void getPressureReading() {
	boolean charging = 1;
	boolean highpressure = 0;
	int old_pressure; // pressures are raw readings ("counts") from HSC sensor
	int new_pressure;

	digitalWrite(READINGLED, HIGH);
	pressure_sensor.update();
	new_pressure = pressure_sensor.pressure_Raw();

	while (charging) {
		old_pressure = new_pressure;
		highpressure = (new_pressure > HIGH_CUTOUT);
		if (highpressure) {
			charging = 0;
			digitalWrite(WARNLED, HIGH);
			break;
			}
		else {
			digitalWrite(WARNLED, LOW);
			}
		if (charging) {
			digitalWrite(MOTOR, HIGH);
			int pump_duration = 50;
			delay(pump_duration);
			digitalWrite(MOTOR, LOW);
			delay(500); // allow for pump motor inertia, and escape of some bubbles
			}
		pressure_sensor.update();
		new_pressure = pressure_sensor.pressure_Raw();
		charging = (abs(new_pressure - old_pressure) > ERROR);
		}
	pressure_sensor.stop();
	printReading();
	digitalWrite(READINGLED, LOW);
	}

void int0_isr() {
	detachInterrupt(0);
	ALARM = true;
	}

void setup() {
	// put your setup code here, to run once:
	setup_pins();
	Serial.begin(115200);
	pressure_sensor.setMinRaw(1638);
	pressure_sensor.setMaxRaw(14746);
	pressure_sensor.setTemperatureCompensated(1);
	Serial.println("Initialising pressure sensor.");
	print_datestamp();
	Serial.println();
	pressure_sensor.update();
	attachInterrupt(0, int0_isr, LOW);
	rtc.writeAlarm(2, alarmModePerMinute, time_buf);
	rtc.setSQIMode(sqiModeAlarm2);
}

void loop() {
	// put your main code here, to run repeatedly:
	if (ALARM) {
		getPressureReading();
		rtc.clearAlarmFlag(3);
		ALARM = false;
		attachInterrupt(0, int0_isr, LOW);
	}
	Serial.flush();
	powerDown();
}
