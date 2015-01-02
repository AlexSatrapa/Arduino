#ifndef PINS
#define PINS

// DIGITAL I/O

// Pin 0 UART RX, PCINT16
// Pin 1 UART TX, PCINT17
// Pin 2 interrupt 0, PCINT18
// Pin 3 interrupt 1, PCINT19, OC2B, PWM output
// Pin 4 PCINT20/XCK/T0
// Pin 5 PCINT21/OC0B/T1, PWM
// Pin 6 PCINT22/OC0A/AIN0, PWM
// Pin 7 PCINT23/AIN1
// Pin 8 PCINT0/CLK0/ICP1
// Pin 9 PCINT1/OC1A, PWM

// SD Card (SPI) uses pins 10, 11, 12, 13
// Pin 10 SPI SS, PWM
// Pin 11 SPI MOSI, PWM
// Pin 12 SPI MISO
// Pin 13 SPI SCK, LED

// ANALOGUE I/O
#define I2C_DA 5 // I2C data
#define I2C_CL 6 // I2C clock

#endif

// PWM notes: http://playground.arduino.cc/Main/TimerPWMCheatsheet
