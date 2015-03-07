#include "pins.h"

uint8_t pwm_rate = 0;
inline void breathe_led(uint8_t pin)
{
    int period = 6000; // 7 seconds, about the period of human breathing
    float fx = 1.0 - sqrt((cos( millis() * 2 * PI / period ) + 1) / 2) * 0.9;
    fx *= fx;
    uint8_t new_rate = trunc(fx * 0xFF);
    if (new_rate != pwm_rate)
    {
        pwm_rate = new_rate;
        analogWrite( pin, pwm_rate );
    }
}

void setup() {
	// put your setup code here, to run once:
	pinMode( LED, OUTPUT );
}

void loop() {
	// put your main code here, to run repeatedly:
	digitalWrite( LED, HIGH );
	delay(1000);
	digitalWrite( LED, LOW );
	delay(1000);
}
