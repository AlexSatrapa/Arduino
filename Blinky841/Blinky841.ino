#include "pins.h"
#include <ChaseLEDs.h>

byte pins[] = {2, 3, 4, 5, 6, 7, 8, 9};
ChaseLEDs chaser(pins, sizeof(pins), 500);

void setup() {
	// put your setup code here, to run once:
}

void loop() {
	// put your main code here, to run repeatedly:
	chaser.loop();
}
