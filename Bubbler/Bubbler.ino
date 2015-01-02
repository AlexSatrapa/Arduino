#include <SPI.h>
#include "pins.h"

void readPressureGauge() {
	digitalWrite(SELECT, LOW);
	byte val0, val1, val2, val3, val4;
	val0 = 0xAA;
	val1 = SPI.transfer(val0);
	val2 = SPI.transfer(val0);
	val3 = SPI.transfer(val0);
	//val4 = SPI.transfer(0x00);
	digitalWrite(SELECT, HIGH);
	Serial.print("Read:");
	Serial.print(val1);
	Serial.print(val2);
	Serial.print(val3);
	//Serial.print(val4, HEX);
	Serial.println(".");
	}

void setup() {
	// put your setup code here, to run once:
	setup_pins();
	Serial.begin(9600);
	// http://arduino.cc/en/Reference/SPI
	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV128); // HSC clock speed is 50-800kHz
}

void loop() {
	// put your main code here, to run repeatedly:
	readPressureGauge();
}
