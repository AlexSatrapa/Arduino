#ifndef PINS
#define PINS

// Arduino Pins
// DIGITAL I/O
// Port D
// D0 UART RX, PCINT16, PD0
// D1 UART TX, PCINT17, PD1
// D2 INT 0, PCINT18, PD2
#define ALARM_PIN 2
// D3 INT 1, PCINT19, PD3, OC2B, PWM output
// D4 PCINT20, XCK, T0, PD4
// D5 PCINT21, OC0B, T1, PD5, PWM
// D6 PCINT22, OC0A, AIN0, PD6, PWM
// D7 PCINT23, AIN1, PD7
#define PRESSURE_SS_PIN 7

// Port B
// D8 PCINT0, CLK0, ICP1, PB0
#define CLOCK_SS_PIN 8
// D9 PCINT1, OC1A, PB1, PWM
// ** SD Card (SPI) uses pins 10, 11, 12, 13
// D10 SPI SS, PCINT2, PB2, PWM
// D11 SPI MOSI, PCINT3, PB3, PWM
// D12 SPI MISO, PCINT4, PB4
// D13 SPI SCK, PCINT5, PB5, LED

// ANALOGUE I/O
// Port C
// A0 PCINT8, ADC0, PC0
// A1 PCINT9, ADC1, PC1
// A2 PCINT10, ADC2, PC2
// A3 PCINT11, ADC3, PC3
// A4 I2C SDA, PCINT12, PC4
// A5 I2C SCL, PCINT13, PC5


// Freetronics LeoStick Pins
// D0 PD2 INT2 RXD1
// D1 PD3 INT3 TXD1
// D2 PD1 INT1 SDA
// D3 PD0 INT0 SCL OC0B
// D4 A6 PD4 ICP1 ADC8
// D5 PC6 OC3A ¬OC4A
// D6 A8 PD7 OC4D ADC10 T0
// D7 PE6 AIN0
// D8 A9 PB4 ADC11
// D9 A10 PB5 OC1A ¬OC4B ADC12 (LED2 Green)
//#define GREEN_LED 9
// D10 A11 PB6 OC1B OC4B ADC13 (LED2 Blue)
//#define BLUE_LED 10
// D11 PB7 OC0A OC1C RTS (Piezo)
//#define PIEZO 11
// D12 PD6 OC4D ADC9
// D13 PC7 ICP3 CLK0 OC4A (LED2 Red)
//#define RED_LED 13

// A0 PF7 ADC7 TDI
// A1 PF6 ADC6 TDO
// A2 PF5 ADC5 TMS
// A3 PF4 ADC4 TCK
// A4 PF1 ADC1
// A5 PF0 ADC0

#endif

// PWM notes: http://playground.arduino.cc/Main/TimerPWMCheatsheet
