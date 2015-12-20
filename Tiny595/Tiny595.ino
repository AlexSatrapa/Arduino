#include "pins.h"
#include <SPI.h>

// Drive a 74HC595 using SPI (MOSI, CLK, SS)
// http://jumptuck.com/2011/11/03/how-to-drive-595-shift-registers-with-avr-hardware-spi/

byte n = 0;
SPISettings spi_settings;

void setup() {
	// put your setup code here, to run once:
	SPI.begin();
	spi_settings = SPISettings(100000, MSBFIRST, SPI_MODE0);
}

void loop() {
	// put your main code here, to run repeatedly:
	SPI.beginTransaction(spi_settings);
	digitalWrite(SS, LOW);
	SPI.transfer(n++);
	digitalWrite(SS, HIGH);
	delay(500);
}
