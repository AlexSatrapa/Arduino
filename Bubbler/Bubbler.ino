#include <SPI.h>
#include "pins.h"
#include "SSC.h"

SSC pressure_sensor(0x00, SELECT);

void printReading( float accumulator ) {
	Serial.print(pressure_sensor.pressure() * 70.3); // PSI to cm of water
	Serial.print(", ");
	Serial.print(pressure_sensor.temperature());
	Serial.print(", ");
	Serial.println(accumulator * 70.3);
	}

void chargePressureTube() {
	digitalWrite(MOTORLED, HIGH);
	float new_pressure = 0.0;
	float alpha = 0.3;
	float accumulator = 0.0;
	boolean charging = 1;
	boolean highpressure = 0;
	new_pressure = pressure_sensor.pressure();
	accumulator = new_pressure;

	while (charging) {
		accumulator = (alpha * new_pressure) + (1.0 - alpha) * accumulator;
		printReading(accumulator);
		highpressure = (new_pressure > (0.9 * pressure_sensor.maxPressure()));
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
			int pump_duration = constrain(trunc(accumulator * 70.0), 25, 100);
			delay(pump_duration);
			digitalWrite(MOTOR, LOW);
			}
		delay(1000); // allow for pump motor inertia, and escape of some bubbles
		pressure_sensor.update();
		new_pressure = pressure_sensor.pressure();
		charging = (abs(new_pressure - accumulator) / accumulator > 0.01);
		}
	digitalWrite(MOTORLED, LOW);
	}

void readPressureGauge() {
	float old_pressure = pressure_sensor.pressure();
	float accumulator = old_pressure;
	float alpha = 0.3;
	boolean stable = 1;
	while (stable) {
		delay(10000);
		digitalWrite(READINGLED, HIGH);
		pressure_sensor.update();
		accumulator = (alpha * pressure_sensor.pressure()) + (1.0 - alpha) * accumulator;
		printReading(accumulator);
		digitalWrite(READINGLED, LOW);
		stable = (abs(old_pressure - accumulator) / old_pressure <= 0.01);
		}
	}

void setup() {
	// put your setup code here, to run once:
	setup_pins();
	Serial.begin(9600);
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
	Serial.println(". o O ( Making Bubbles )");
	chargePressureTube();
	Serial.println(". o O ( Reading Pressure )");
	readPressureGauge();
}
