/*
 * Copyright (C) 2012 Southern Storm Software, Pty Ltd.
 *
 * MODIFIED (2013) to use the Arduino Time.h library functions; http://playground.arduino.cc/Code/Time
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
This example demonstrates how to read and write time, date, and other
information from the DS3232 realtime clock chip.  The application is
controlled from the Serial Monitor in the Arduino IDE using a simple
command and response system.  Configure the Serial Monitor to use
"Newline" line endings.  Type "HELP" to get a list of commands.

How to wire the Freetronics RTC module
--------------------------------------
Freetronics RTC -> Freetronics Eleven
    GND         ->     GND
    VCC         ->     5V
    SCL         ->     D5
    SDA         ->     D4
    BAT         ->     not connected
    32K         ->     not connected
    SQI         ->     D2 (this is INT0 on UNO boards)
    RST         ->     not connected
*/

#include <Wire.h>
#include <SPI.h>
#include <Time.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <DS3232RTC.h>  // DS3232 library that returns time as a time_t
#include <DS3234.h>
#include <RTC.h>
#include "pins.h"

DS3232RTC DS3232 = DS3232RTC();
DS3234RTC DS3234 = DS3234RTC(DS3234_SS_PIN);

char buffer[64];
size_t buflen;
bool led_on = false;
volatile bool ds3232_alarmed = false;
volatile bool ds3234_alarmed = false;

const char *days[] = {
    "Sun, ", "Mon, ", "Tue, ", "Wed, ", "Thu, ", "Fri, ", "Sat, "
};

const char *months[] = {
    " Jan ", " Feb ", " Mar ", " Apr ", " May ", " Jun ",
    " Jul ", " Aug ", " Sep ", " Oct ", " Nov ", " Dec "
};

static uint8_t monthLengths[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

inline bool isLeapYear(unsigned int year)
{
    if ((year % 100) == 0)
        return (year % 400) == 0;
    else
        return (year % 4) == 0;
}

inline uint8_t monthLength(const tmElements_t *date)
{
    if (date->Month != 2 || !isLeapYear(date->Year))
        return monthLengths[date->Month - 1];
    else
        return 29;
}

inline void print_binary(uint8_t value)
{
    static const char binchars[] = "01";
    Serial.print(binchars[(value >> 7) & 0x01]);
    Serial.print(binchars[(value >> 6) & 0x01]);
    Serial.print(binchars[(value >> 5) & 0x01]);
    Serial.print(binchars[(value >> 4) & 0x01]);
    Serial.print(binchars[(value >> 3) & 0x01]);
    Serial.print(binchars[(value >> 2) & 0x01]);
    Serial.print(binchars[(value >> 1) & 0x01]);
    Serial.println(binchars[value & 0x01]);
}

void cmdHelp(const char *);
void ds3232Alarm();
void ds3234Alarm();
void showTrigger();
void printDec2(int value);
void printProgString(const char *str);
void processCommand(const char *buf);
bool matchString(const char *name, const char *str, int len);

void setup() {
    Serial.begin(115200);
    SPI.begin();

    buflen = 0;
    cmdHelp(0);

    DS3232.set33kHzOutput(false);

    // Wire SQI pin to pin 2 on Uno, Ethernet & Mega; pin 3 on Leonardo
    // See: http://www.arduino.cc/en/Reference/AttachInterrupt
    DS3232.clearAlarmFlag(3);  // 3 is both (1+2)
    ds3232_alarmed = false;
    attachInterrupt(1, ds3232Alarm, FALLING);

    uint8_t flags[5] = { 0, 1, 1, 1, 1 };
    SPI.beginTransaction(DS3234.spi_settings);
    DS3234_set_a1(DS3234_SS_PIN, 00, 10, 22, 0, flags);
    DS3234_clear_a1f(DS3234_SS_PIN);
    SPI.endTransaction();
    attachInterrupt(0, ds3234Alarm, FALLING);
}

void loop() {
    if (ds3232_alarmed or ds3234_alarmed) showTrigger();

    if (Serial.available()) {
        // Process serial input for commands from the host.
        int ch = Serial.read();
        if (ch == 0x0A || ch == 0x0D) {
            // End of the current command.  Blank lines are ignored.
            if (buflen > 0) {
                buffer[buflen] = '\0';
                buflen = 0;
                processCommand(buffer);
            }
        } else if (ch == 0x08) {
            // Backspace over the last character.
            if (buflen > 0)
                --buflen;
        } else if (buflen < (sizeof(buffer) - 1)) {
            // Add the character to the buffer after forcing to upper case.
            if (ch >= 'a' && ch <= 'z')
                buffer[buflen++] = ch - 'a' + 'A';
            else
                buffer[buflen++] = ch;
        }
    }
}

void ds3232Alarm()  // Triggered when DS3232 alarm is fired
{
    ds3232_alarmed = true;
}

void ds3234Alarm() // Triggered when DS3234 alarm is fired
{
    ds3234_alarmed = true;
}

void showTrigger()
{
    if(ds3232_alarmed) {
         if (DS3232.isAlarmFlag(1)) {
              Serial.println("DS3232 alarm 1 triggered");
              DS3232.clearAlarmFlag(1);
         }
         if (DS3232.isAlarmFlag(2)) {
              Serial.println("DS3232 alarm 2 triggered");
              DS3232.clearAlarmFlag(2);
         }
         ds3232_alarmed = false;
    }
    if(ds3234_alarmed) {
        if(DS3234_triggered_a1(DS3234_SS_PIN)) {
            Serial.println("DS3234 alarm 1 triggered");
            DS3234_clear_a1f(DS3234_SS_PIN);
        }
        if(DS3234_triggered_a2(DS3234_SS_PIN)) {
            Serial.println("DS3234 alarm 2 triggered");
            DS3234_clear_a2f(DS3234_SS_PIN);
        }
        ds3234_alarmed = false;
    }
}

void printDec2(int value)
{
    Serial.print((char)('0' + (value / 10)));
    Serial.print((char)('0' + (value % 10)));
}

void printProgString(const char *str)
{
    for (;;) {
        char ch = (char)(pgm_read_byte(str));
        if (ch == '\0')
            break;
        Serial.print(ch);
        ++str;
    }
}

byte readField(const char *args, int &posn, int maxValue)
{
    int value = -1;
    if (args[posn] == ':' && posn != 0)
        ++posn;
    while (args[posn] >= '0' && args[posn] <= '9') {
        if (value == -1)
            value = 0;
        value = (value * 10) + (args[posn++] - '0');
        if (value > 99)
            return 99;
    }
    if (value == -1 || value > maxValue)
        return 99;
    else
        return value;
}

// "TIME" command.
void cmdTime(const char *args)
{
    tmElements_t tm;

    if (*args != '\0') {
        // Set the current time.
        int posn = 0;
        tm.Hour = readField(args, posn, 23);
        tm.Minute = readField(args, posn, 59);
        if (args[posn] != '\0')
            tm.Second = readField(args, posn, 59);
        else
            tm.Second = 0;
        if (tm.Hour == 99 || tm.Minute == 99 || tm.Second == 99) {
            Serial.println("Invalid time format; use HH:MM:SS");
            return;
        }
        DS3232.writeTime(tm);
        DS3234.writeTime(tm);
        Serial.print("Time has been set to: ");
    }

    // Read the current time.
    DS3232.read(tm);
    printDec2(tm.Hour);
    Serial.print(':');
    printDec2(tm.Minute);
    Serial.print(':');
    printDec2(tm.Second);
    Serial.println();

    // Read the current time.
    DS3234.read(tm);
    printDec2(tm.Hour);
    Serial.print(':');
    printDec2(tm.Minute);
    Serial.print(':');
    printDec2(tm.Second);
    Serial.println();
}

// "DATE" command.
void cmdDate(const char *args)
{
    tmElements_t tm;

    if (*args != '\0') {
        // Set the current date.
        unsigned long value = 0;
        while (*args >= '0' && *args <= '9')
            value = value * 10 + (*args++ - '0');
        if (value < 20000000 || value >= 22000000) {
            Serial.println("Year must be between 2000 and 2199");
            return;
        }
        tm.Day = (byte)(value % 100);
        tm.Month = (byte)((value / 100) % 100);
        tm.Year = CalendarYrToTm( (unsigned int)(value / 10000) );
        if (tm.Month < 1 || tm.Month > 12) {
            Serial.println("Month must be between 1 and 12");
            return;
        }
        uint8_t len = monthLength(&tm);
        if (tm.Day < 1 || tm.Day > len) {
            Serial.print("Day must be between 1 and ");
            Serial.println(len, DEC);
            return;
        }
        tm.Hour = 0;
        tm.Minute = 0;
        tm.Second = 0;
        tm.Wday = 0; // Ask Time library to calculate day of week.
        DS3232.writeDate(tm);
        DS3234.writeDate(tm);
        Serial.print("Date has been set to: ");
    } /* */

    // Read the current date.
    DS3232.read(tm);
    if (tm.Wday > 0) Serial.print(days[tm.Wday - 1]);
    Serial.print(tm.Day, DEC);
    Serial.print(months[tm.Month - 1]);
    Serial.println(tmYearToCalendar(tm.Year), DEC);  // NB! Remember tmYearToCalendar()

    DS3234.read(tm);
    if (tm.Wday > 0) Serial.print(days[tm.Wday - 1]);
    Serial.print(tm.Day, DEC);
    Serial.print(months[tm.Month - 1]);
    Serial.println(tmYearToCalendar(tm.Year), DEC);  // NB! Remember tmYearToCalendar() */
}

// "TEMP" command.
void cmdTemp(const char *args)
{
    tpElements_t tp;
    DS3232.readTemperature(tp);
    if (tp.Temp != NO_TEMPERATURE) {
        float temp = tp.Temp + (tp.Decimal / 100);
        Serial.print(temp);
        Serial.print("'C (");
        Serial.print(temperatureCToF(temp));
        Serial.println("'F)");
    } else {
        Serial.println("Temperature is not available");
    }
    DS3234.readTemperature(tp);
    float temp = tp.Temp + (tp.Decimal / 100);
    Serial.print(temp);
    Serial.print("'C (");
    Serial.print(temperatureCToF(temp));
    Serial.println("'F)");
}

void printAlarm(byte alarmNum, const alarmMode_t mode, const tmElements_t time, bool ison)
{
    Serial.print("Alarm ");
    Serial.print(alarmNum, DEC);

    Serial.print(": INT ");
    if (ison) {
        Serial.print("ON");
    } else {
        Serial.print("OFF");
    }
    Serial.print(" & TRIG ");

    if (mode == alarmModeOff) {
        Serial.println("OFF");
        return;
    }
    Serial.print("Every ");
    switch (mode) {
        case alarmModePerSecond:
            Serial.print("Second");
            break;
        case alarmModePerMinute:
            Serial.print("Minute");
            break;
        case alarmModeSecondsMatch:
            Serial.print("??:??:");
            printDec2(time.Second);
            break;
        case alarmModeMinutesMatch:
            Serial.print("??:");
            printDec2(time.Minute);
            Serial.print(":");
            printDec2(time.Second);
            break;
        case alarmModeHoursMatch:
        case alarmModeDateMatch:
        case alarmModeDayMatch:
            if (mode == alarmModeDateMatch) {
              Serial.print(time.Day);
              Serial.print("st/nd/rd/th, at ");
            }
            if (mode == alarmModeDayMatch) {
              Serial.print(days[time.Wday - 1]);
              Serial.print(", at ");
            }
            printDec2(time.Hour);
            Serial.print(":");
            printDec2(time.Minute);
            Serial.print(":");
            printDec2(time.Second);
            break;
        default:
            Serial.print("Unknown");
            break;
    }
    Serial.println();
}

// "ALARMS" command.
void cmdAlarms(const char *args)
{
    tmElements_t time;
    alarmMode_t mode;
    bool ison;
    Serial.println("DS3232:");
    for (byte alarmNum = 1; alarmNum <= 2; ++alarmNum) {
        DS3232.readAlarm(alarmNum, mode, time);
        ison = DS3232.isAlarmInterrupt(alarmNum);
        printAlarm(alarmNum, mode, time, ison);
    }
    Serial.println("DS3234:");
    for (byte alarmNum = 1; alarmNum <= 2; ++alarmNum) {
        DS3234.readAlarm(alarmNum, mode, time);
        ison = DS3234.isAlarmInterrupt(alarmNum);
        printAlarm(alarmNum, mode, time, ison);
    }
    showTrigger();

    if (*args != '\0')
    {
        Serial.println("Clearing alarm flags");
        DS3232.clearAlarmFlag(1);
        DS3232.clearAlarmFlag(2);
        DS3234.clearAlarmFlag(1);
        DS3234.clearAlarmFlag(2);
    }
}

const char s_OFF[] PROGMEM = "OFF";

// "ALARM" command.
void cmdAlarm(const char *args)
{
    tmElements_t time;
    alarmMode_t mode;
    bool ison;

    memset(&time, 0, sizeof(tmElements_t));

    int posn = 0;
    byte alarmNum = readField(args, posn, 2);
    if (!alarmNum || alarmNum == 99) {
        Serial.println("Alarm number must be 1 or 2");
        return;
    }
    while (args[posn] == ' ' || args[posn] == '\t')
        ++posn;
    if (args[posn] != '\0') {
        // Set the alarm to a new value.
        if (matchString(s_OFF, args + posn, strlen(args + posn))) {
            mode = alarmModeOff;
        } else {
            time.Hour = readField(args, posn, 23);
            time.Minute = readField(args, posn, 59);
            if (time.Hour == 99 || time.Minute == 99) {
                Serial.println("Invalid alarm time format; use HH:MM");
                return;
            }
            mode = alarmModeHoursMatch;
            bool a1 = DS3232.isAlarmInterrupt(1);
            bool a2 = DS3232.isAlarmInterrupt(2);
            if (alarmNum == 1) {
              a1 = true;
            } else {
              a2 = true;
            }
            if (a1 && a2) {
              DS3232.setSQIMode(sqiModeAlarmBoth);
            } else if (a1) {
              DS3232.setSQIMode(sqiModeAlarm1);
            } else if (a2) {
              DS3232.setSQIMode(sqiModeAlarm2);
            }
        }
        DS3232.writeAlarm(alarmNum, mode, time);
    }

    // Print the current state of the alarm.
    Serial.println("DS3232:");
    DS3232.readAlarm(alarmNum, mode, time);
    ison = DS3232.isAlarmInterrupt(alarmNum);
    printAlarm(alarmNum, mode, time, ison);
    Serial.println("DS3234:");
    DS3234.readAlarm(alarmNum, mode, time);
    ison = DS3234.isAlarmInterrupt(alarmNum);
    printAlarm(alarmNum, mode, time, ison);
}

// "SRAM" command.
void cmdSram(const char *args)
{
    static const char hexchars[] = "0123456789ABCDEF";
    SRAM.flush();  // reset cursor
    int count = SRAM.available();
    for (int offset = 0; offset < count; ++offset) {
        if ((offset % 16) == 0) {
            if (offset)
                Serial.println();
            Serial.print(hexchars[(offset >> 12) & 0x0F]);
            Serial.print(hexchars[(offset >> 8) & 0x0F]);
            Serial.print(hexchars[(offset >> 4) & 0x0F]);
            Serial.print(hexchars[offset & 0x0F]);
            Serial.print(':');
            Serial.print(' ');
        }
        byte value = SRAM.read();
        Serial.print(hexchars[(value >> 4) & 0x0F]);
        Serial.print(hexchars[value & 0x0F]);
        Serial.print(' ');
    }
    Serial.println();
}

// "DUMP" command
void cmdDump(const char *args)
{
  int value = -1;

  if (*args != '\0') {
    while (*args >= '0' && *args <= '9') {
      if (value == -1) value = 0;
      value = value * 10 + (*args++ - '0');
    }
  }
  if (value < 0 || value > 1) {
    Serial.println("Number must be 0 or 1");
      return;
  }

  SRAM.flush();  // reset cursor
  if (value == 0) {
    // Correct way is to write until available() == 0
    uint8_t i = 0;
    while (SRAM.available()) SRAM.write( i );
  } else {
    // SRAM is from register 14h (20) to FFh (255)
    for (int i = 20; i < 256; i++) SRAM.write( (uint8_t)i );
  }
  Serial.println("OK");
}

// "REGS" command
void cmdRegisters(const char *)
{
    byte value;
    value = DS3232.readControlRegister();
    Serial.write("Ctrl: ");
    print_binary( value );
    value = DS3232.readStatusRegister();
    Serial.write("Stat: ");
    print_binary( value );
}

void cmdMap(const char *)
{
    static const char hexchars[] = "0123456789ABCDEF";
    uint8_t value;

    Wire.beginTransmission(DS3232_I2C_ADDRESS);
    Wire.write(0x00);  // sends 00h - Seconds register
    Wire.endTransmission();

    Wire.requestFrom(DS3232_I2C_ADDRESS, 0x14);

    for (int offset = 0; offset < 0x14; offset++) {
        value = Wire.read();

        if (offset % 7 == 0) {
            if (offset != 0) Serial.println();
            Serial.print(hexchars[(offset >> 4) & 0x0F]);
            Serial.print(hexchars[offset & 0x0F]);
            Serial.print(':');
            Serial.print(' ');
        }

        Serial.print(hexchars[(value >> 4) & 0x0F]);
        Serial.print(hexchars[value & 0x0F]);
        Serial.print(' ');
    }
    Serial.println();
}

// List of all commands that are understood by the sketch.
typedef void (*commandFunc)(const char *args);
typedef struct
{
    const char *name;
    commandFunc func;
    const char *desc;
    const char *args;
} command_t;
const char s_cmdTime[] PROGMEM = "TIME";
const char s_cmdTimeDesc[] PROGMEM =
    "Read or write the current time";
const char s_cmdTimeArgs[] PROGMEM = "[HH:MM:SS]";
const char s_cmdDate[] PROGMEM = "DATE";
const char s_cmdDateDesc[] PROGMEM =
    "Read or write the current date";
const char s_cmdDateArgs[] PROGMEM = "[YYYYMMDD]";
const char s_cmdTemp[] PROGMEM = "TEMP";
const char s_cmdTempDesc[] PROGMEM =
    "Read the current temperature";
const char s_cmdAlarms[] PROGMEM = "ALARMS";
const char s_cmdAlarmsDesc[] PROGMEM =
    "Print the status of all alarms";
const char s_cmdAlarm[] PROGMEM = "ALARM";
const char s_cmdAlarmDesc[] PROGMEM =
    "Read or write a specific alarm";
const char s_cmdAlarmArgs[] PROGMEM = "NUM [HH:MM|OFF]";
const char s_cmdSram[] PROGMEM = "SRAM";
const char s_cmdSramDesc[] PROGMEM =
    "Print the contents of SRAM";
const char s_cmdDump[] PROGMEM = "DUMP";
const char s_cmdDumpDesc[] PROGMEM =
    "Write contents into SRAM; 0=Zeros, 1=Counter";
const char s_cmdDumpArgs[] PROGMEM = "(0|1)";
const char s_cmdRegisters[] PROGMEM = "REGS";
const char s_cmdRegistersDesc[] PROGMEM =
    "Show the Control and Status registers";
const char s_cmdMap[] PROGMEM = "MAP";
const char s_cmdMapDesc[] PROGMEM =
    "Print the content of Address Map";
const char s_cmdHelp[] PROGMEM = "HELP";
const char s_cmdHelpDesc[] PROGMEM =
    "Prints this help message";
const command_t commands[] PROGMEM = {
    {s_cmdTime, cmdTime, s_cmdTimeDesc, s_cmdTimeArgs},
    {s_cmdDate, cmdDate, s_cmdDateDesc, s_cmdDateArgs},
    {s_cmdTemp, cmdTemp, s_cmdTempDesc, 0},
    {s_cmdAlarms, cmdAlarms, s_cmdAlarmsDesc, 0},
    {s_cmdAlarm, cmdAlarm, s_cmdAlarmDesc, s_cmdAlarmArgs},
    {s_cmdSram, cmdSram, s_cmdSramDesc, 0},
    {s_cmdDump, cmdDump, s_cmdDumpDesc, s_cmdDumpArgs},
    {s_cmdRegisters, cmdRegisters, s_cmdRegistersDesc, 0},
    {s_cmdMap, cmdMap, s_cmdMapDesc, 0},
    {s_cmdHelp, cmdHelp, s_cmdHelpDesc, 0},
    {0, 0}
};

// "HELP" command.
void cmdHelp(const char *)
{
    int index = 0;
    for (;;) {
        const char *name = (const char PROGMEM *)
            (pgm_read_word(&(commands[index].name)));
        if (!name)
            break;
        const char *desc = (const char PROGMEM *)
            (pgm_read_word(&(commands[index].desc)));
        const char *args = (const char PROGMEM *)
            (pgm_read_word(&(commands[index].args)));
        printProgString(name);
        if (args) {
            Serial.print(' ');
            printProgString(args);
        }
        Serial.println();
        Serial.print("    ");
        printProgString(desc);
        Serial.println();
        ++index;
    }
}

// Match a data-space string where the name comes from PROGMEM.
bool matchString(const char *name, const char *str, int len)
{
    for (;;) {
        char ch1 = (char)(pgm_read_byte(name));
        if (ch1 == '\0')
            return len == 0;
        else if (len == 0)
            break;
        if (ch1 >= 'a' && ch1 <= 'z')
            ch1 = ch1 - 'a' + 'A';
        char ch2 = *str;
        if (ch2 >= 'a' && ch2 <= 'z')
            ch2 = ch2 - 'a' + 'A';
        if (ch1 != ch2)
            break;
        ++name;
        ++str;
        --len;
    }
    return false;
}

// Process commands from the host.
void processCommand(const char *buf)
{
    // Skip white space at the start of the command.
    while (*buf == ' ' || *buf == '\t')
        ++buf;
    if (*buf == '\0')
        return;     // Ignore blank lines.

    // Extract the command portion of the line.
    const char *cmd = buf;
    int len = 0;
    for (;;) {
        char ch = *buf;
        if (ch == '\0' || ch == ' ' || ch == '\t')
            break;
        ++buf;
        ++len;
    }

    // Skip white space after the command name and before the arguments.
    while (*buf == ' ' || *buf == '\t')
        ++buf;

    // Find the command and execute it.
    int index = 0;
    for (;;) {
        const char *name = (const char PROGMEM *)
            (pgm_read_word(&(commands[index].name)));
        if (!name)
            break;
        if (matchString(name, cmd, len)) {
            commandFunc func =
                (commandFunc)(pgm_read_word(&(commands[index].func)));
            (*func)(buf);
            return;
        }
        ++index;
    }

    // Unknown command.
    Serial.println("Unknown command, valid commands are:");
    cmdHelp(0);
}
