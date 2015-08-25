#include "pins.h"

volatile unsigned long button_pressed_millis = 1;
const unsigned long led_duration = 500; // milliseconds to stay lit

ISR(PCINT0_vect) {
	digitalWrite(LED, HIGH);
	button_pressed_millis = millis();
}

void setup() {
	// put your setup code here, to run once:
	PCMSK0 |= B00000010; // Enable PCINT1
	GIMSK |= B00010000; // Enable PCI group 0
	digitalWrite(LED, HIGH);
	pinMode(LED, OUTPUT);
	pinMode(BUTTON, INPUT_PULLUP);
	button_pressed_millis = 1;
}

void loop() {
	// put your main code here, to run repeatedly:
	if (button_pressed_millis > 0 and (millis() > button_pressed_millis + led_duration)) {
		digitalWrite(LED, LOW);
		button_pressed_millis = 0;
	}
}

// TODO: only light LED when pin is transitioning from HIGH to LOW (i.e.: pressed, not released)
// https://github.com/GreyGnome/EnableInterrupt/blob/master/EnableInterrupt.h
