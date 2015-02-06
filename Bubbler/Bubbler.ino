#include <SPI.h>
#include "pins.h"
#include <SSC.h>

SSC pressure_sensor(0x00, SELECT);
int MIN_RAW = 1638;
int MAX_RAW = 14746;
int ERROR = (MAX_RAW - MIN_RAW) / 400; // 0.25%

void printReading() {
	Serial.print(pressure_sensor.pressure() * 70.3); // PSI to cm of water
	Serial.print(", ");
	Serial.println(pressure_sensor.temperature());
	}

void getPressureReading() {
	float new_pressure = 0.0;
	float alpha = 0.3;
	float accumulator = 0.0;
	boolean charging = 1;
	boolean highpressure = 0;
	pressure_sensor.update();
	new_pressure = pressure_sensor.pressure_Raw();
	accumulator = new_pressure;

	while (charging) {
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
			int pump_duration = trunc(accumulator / 40);
			delay(pump_duration);
			digitalWrite(MOTOR, LOW);
			delay(500); // allow for pump motor inertia, and escape of some bubbles
			}
		pressure_sensor.update();
		new_pressure = pressure_sensor.pressure_Raw();
		accumulator = (alpha * new_pressure) + (1.0 - alpha) * accumulator;
		charging = (abs(new_pressure - accumulator) > ERROR);
		}
	printReading();
	}

void setup() {
	// put your setup code here, to run once:
	setup_pins();
	Serial.begin(115200);
	pressure_sensor.setMinRaw(1638);
	pressure_sensor.setMaxRaw(14746);
	pressure_sensor.setMinPressure(0.0);
	pressure_sensor.setMaxPressure(5.0);
	pressure_sensor.setTemperatureCompensated(1);
	Serial.print("Initialising pressure sensor: ");
	Serial.println(pressure_sensor.start());
	Serial.print("Maximum pressure is ");
	Serial.println(pressure_sensor.maxPressure() * 70.3);
	pressure_sensor.update();
}

void loop() {
	// put your main code here, to run repeatedly:
	getPressureReading();
	delay(60000);
}
