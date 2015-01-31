#ifndef PINS
#define PINS

// Arduino Pins
// DIGITAL I/O
// Pin 0 UART RX, PCINT16
// Pin 1 UART TX, PCINT17
// Pin 2 interrupt 0, PCINT18
#define DS3231_ALARM 2 // Stalker: solder the jumper!
// Pin 3 interrupt 1, PCINT19, OC2B, PWM output
// RST/32K allows the DS3231 Reset or 32kHz output to be connected to PD3/INT1
//#define DS3231_RESET 3 // Stalker: solder the jumper!
//#define DS3231_32K 3
// Pin 4 PCINT20/XCK/T0
#define POWER_TF 4 // microSD card power on Pin 4. Solder the jumper!
// Pin 5 PCINT21/T1/OC0B
#define POWER_BEE 5 // Allow Pin 5 to control XBee power supply. Solder the jumper!
// Pin 6 PCINT22/OC0A/AIN0, PWM
// Pin 7 PCINT23/AIN1
// Pin 8 PCINT0/CLK0/ICP1
// Pin 9 PCINT1/OC1A, PWM

// SD Card (SPI) uses pins 10, 11, 12, 13
// Pin 10 SPI SS, PWM
// Pin 11 SPI MOSI, PWM
// Pin 12 SPI MISO
// Pin 13 SPI SCK, LED
#define LED 13

// ANALOGUE I/O
//#define I2C_DA 5 // I2C data
//#define I2C_CL 6 // I2C clock

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


// Seeeduino Stalker special pins
// POWER_BEE allows the XBEE power to be controlled using pin 5. By default it is disabled
// CS_TF jumper selects microSD SPI Slave/Chip Select between D9/PB1 and D10/PB2, by default D10/PB2
//#define CS_TF 9 // cut D10 bridge, solder the jumper
#define CS_TF 10

#endif

// PWM notes: http://playground.arduino.cc/Main/TimerPWMCheatsheet
