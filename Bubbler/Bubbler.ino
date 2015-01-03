#include <SPI.h>
#include "pins.h"

void chargePressureTube() {
	digitalWrite(MOTORLED, HIGH);
	digitalWrite(MOTOR, HIGH);
	delay(500);
	digitalWrite(MOTOR, LOW);
	digitalWrite(MOTORLED, LOW);
	}

void interpretReading(byte *val1, byte *val2, byte *val3, byte *val4) {
	char raw_data_string[13];
	sprintf(raw_data_string, "[%02X:%02X:%02X:%02X]", *val1, *val2, *val3, *val4);
	Serial.print(raw_data_string);
	Serial.print(", ");
	// Status
	byte status = *val1;
	status = status & 0xC0;
	status = status >> 6;
	Serial.print(status, HEX);
	Serial.print(", ");

	// Data
	byte topByte = *val1;
	byte bottomByte = *val2;
	topByte = topByte & 0x3F;
	int hsc_output = topByte;
	hsc_output = hsc_output << 8;
	hsc_output = hsc_output | bottomByte;
	Serial.print(hsc_output);
	Serial.print(", ");

	const int max_counts = 14746;
	const int min_counts = 1638;
	float psi = (float(hsc_output - min_counts) * 5.0) / (max_counts - min_counts);
	float cm_water = psi * 70.3; // PSI to cm of water
	Serial.print(cm_water, 1);
	Serial.print(", ");

	// Temperature correction
	topByte = *val3;
	bottomByte = *val4;
	int raw_temp = topByte;
	raw_temp = raw_temp << 3;
	bottomByte = bottomByte & 0xE0;
	bottomByte = bottomByte >> 5;
	raw_temp = raw_temp | bottomByte;
	float temperature = float(raw_temp) * 200.0;
	temperature = temperature / 2047 - 50.0;
	Serial.print(temperature, 1);
	Serial.println("");
	}

void readPressureGauge() {
	digitalWrite(SELECT, LOW);
	digitalWrite(READINGLED, HIGH);
	byte val0, val1, val2, val3, val4;
	delay(50);
	val0 = 0xAA;
	val1 = SPI.transfer(val0);
	val2 = SPI.transfer(val0);
	val3 = SPI.transfer(val0);
	val4 = SPI.transfer(val0);
	delay(50);
	digitalWrite(SELECT, HIGH);
	interpretReading( &val1, &val2, &val3, &val4 );
	digitalWrite(READINGLED, LOW);
	}

void setup() {
	// put your setup code here, to run once:
	setup_pins();
	Serial.begin(9600);
	// http://arduino.cc/en/Reference/SPI
	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV64); // HSC clock speed is 50-800kHz
}

void loop() {
	// put your main code here, to run repeatedly:
	Serial.println(". o O ( Making Bubbles )");
	chargePressureTube();
	for (int i=0; i<10; i++){
		readPressureGauge();
		delay(200);
	}
	delay(4000);
}
