#include "pins.h"
#include <BlinkLED.h>

BlinkLED blinker(10, 250, 500);

void setup() {
	// put your setup code here, to run once:
}

void loop() {
	// put your main code here, to run repeatedly:
	blinker.loop();
}
