#include <SPI.h>
#include "pins.h"
#include <SSC.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>

SSC pressure_sensor(0x00, SELECT);
RTC_DS1307 RTC;
int MIN_RAW = 1638;
int MAX_RAW = 14746;
int ERROR = (MAX_RAW - MIN_RAW) / 400; // 0.25%

void print_datestamp() {
	DateTime now;
	now = RTC.now();
	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(' ');
	if (now.hour() < 10) {
		Serial.print('0');
		}
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	if (now.minute() < 10) {
		Serial.print('0');
		}
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	if (now.second() < 10) {
		Serial.print('0');
		}
	Serial.print(now.second(), DEC);
	}

void printReading() {
	print_datestamp();
	Serial.print(", ");
	Serial.print(pressure_sensor.pressure());
	Serial.print(", ");
	Serial.println(pressure_sensor.temperature());
	}

/*
How a reading is taken:

1. Start off with a pressure reading.

2. Blow some air into the tube.

3. If the pressure in the tube doesn't change significantly (i.e.: by more than the margin of error of the sensor), consider the tube evacuated, and the air pressure to be at equilibrium with the water pressure.

4. If the pressure does change, go back to step 1.

There is some finessing required in order to ensure that each "blow" is likely to change the overall pressure.
*/
void getPressureReading() {
	boolean charging = 1;
	boolean highpressure = 0;
	int old_pressure; // pressures are raw readings ("counts") from HSC sensor
	int new_pressure;

	pressure_sensor.update();
	new_pressure = pressure_sensor.pressure_Raw();

	while (charging) {
		old_pressure = new_pressure;
		highpressure = (new_pressure > (0.9 * MAX_RAW));
		if (highpressure) {
			charging = 0;
			Serial.println("HIGH PRESSURE!");
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
	printReading();
	}

void setup() {
	// put your setup code here, to run once:
	setup_pins();
	Wire.begin();
	RTC.begin();
	Serial.begin(115200);
	pressure_sensor.setMinRaw(1638);
	pressure_sensor.setMaxRaw(14746);
	pressure_sensor.setMinPressure(0.0);
	pressure_sensor.setMaxPressure(351.5); // cm H20, instead of PSI
	pressure_sensor.setTemperatureCompensated(1);
	Serial.print("Initialising pressure sensor: ");
	Serial.println(pressure_sensor.start());
	Serial.print("Maximum pressure is ");
	Serial.println(pressure_sensor.maxPressure());
	Serial.println("UTC, pressure (cmH20), temperature (C)");
	pressure_sensor.update();
}

void loop() {
	// put your main code here, to run repeatedly:
	getPressureReading();
	delay(60000);
}
