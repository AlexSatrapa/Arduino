#include <Sleep.h>

void powerDown()
{
    // Turn off the analog to digital converter.
    ADCSRA &= ~(1 << ADEN);
    power_adc_disable();

    // Turn off the watchdog timer.
    wdt_disable();

    // Put the device to sleep, including turning off the Brown Out Detector.
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_enable();
#if defined(sleep_bod_disable)
    sleep_bod_disable();
#endif
    sei();
    sleep_cpu();
    sleep_disable();
    sei();

    // Turn the analog to digital converter back on.
    power_adc_enable();
    ADCSRA |= (1 << ADEN);
}
