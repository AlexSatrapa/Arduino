#include "pins.h"
#include <SPI.h>
#include <ChaseLEDs.h>

SPISettings spi_settings;

// Chaser LEDs just to show that the Tiny is alive.
byte pins[] = {2, 7, 8, 9, 10, 9, 8, 7};
ChaseLEDs chaser(pins, sizeof(pins), 500);

const unsigned long advanceTime = 500;
unsigned long lastChange;
unsigned count;

void sendCount() {
	byte countBuf[4];
	countBuf[0] = count / 1000;
	countBuf[1] = (count / 100) % 10;
	countBuf[2] = (count / 10) % 10;
	countBuf[3] = count % 10;
	digitalWrite(SS, LOW); //Drive the CS pin low to select OpenSegment
	SPI.transfer(0x79); // Move cursor …
	SPI.transfer(0x00); // … to position 0
	for(byte x = 0 ; x < 4 ; x++) {
		delay(1); //Small delay between bytes so S7S can buffer them
		SPI.transfer(countBuf[x]); //Send a character from the array out over I2C
	}
	digitalWrite(SS, HIGH); //Release the CS pin to de-select OpenSegment
}

void setup() {
	// put your setup code here, to run once:
	pinMode(SS, OUTPUT);
	digitalWrite(SS, HIGH);
	spi_settings = SPISettings(200000, MSBFIRST, SPI_MODE0);
	SPI.begin();
	SPI.beginTransaction(spi_settings);
	digitalWrite(SS, LOW);
	SPI.transfer('v');
	SPI.endTransaction();
	digitalWrite(SS, HIGH);
	sendCount();
}

void loop() {
	// put your main code here, to run repeatedly:
	unsigned millibuf = millis();
	chaser.loop();
	if (millibuf - lastChange > advanceTime) {
		sendCount();
		if(count++ > 9999) count = 0;
		lastChange = millibuf;
	}
}
