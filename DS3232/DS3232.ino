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
Freetronics RTC -> Arduino Uno
    GND         ->     GND
    VCC         ->     5V
    SCL         ->     A5
    SDA         ->     A4
    BAT         ->     not connected
    32K         ->     not connected
    SQI         ->     D3 (this is INT1 on Uno boards)
    RST         ->     not connected

How to wire the SparkFun DeadOn RTC module
------------------------------------------
SparkFun DeadOn -> Arduino Uno
    GND         -> GND
    VCC         -> 5V
    SQW         -> D2 (this is INT0 on Uno boards)
    CLK         -> D13
    MISO        -> D12
    MOSI        -> D11
    SS          -> D8
*/

#include <avr/pgmspace.h>
#include <string.h>
//#include <DS3232.h>
#include <DS3234.h>
#include <Time.h>
#include "pins.h"

DS3234 RTC = DS3234(CLOCK_SS_PIN);

char buffer[64];
size_t buflen;
bool led_on = false;
alarmMode_t alarmMode = alarmModeUnknown;
dsrtc_calendar_t alarmSetting;
volatile bool alarm_triggered = false;

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

inline uint8_t monthLength(const dsrtc_calendar_t *date)
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
void cmdRegisters(const char *);
void rtcAlarm();
void showTrigger();
void printDec2(int value);
void printTime(dsrtc_calendar_t tm);
void printProgString(const char *str);
void processCommand(const char *buf);
bool matchString(const char *name, const char *str, int len);

void setup() {
    Serial.begin(115200);

    alarmSetting.Year = 15; // type only supports 255, RTC only supports 0-99
    alarmSetting.Month = 3;
    alarmSetting.Wday = 1;
    alarmSetting.Day = 7;
    alarmSetting.Hour = 21;
    alarmSetting.Minute = 43;
    alarmSetting.Second = 57;

    RTC.set33kHzOutput(false);
    RTC.setSQIMode(sqiModeNone);
    RTC.writeAlarm(1, alarmModeSecondsMatch, alarmSetting);
    alarmSetting.Second = 15;
    RTC.writeAlarm(2, alarmModePerMinute, alarmSetting);
    attachInterrupt(0, rtcAlarm, FALLING);
    RTC.clearAlarmFlag(3);
    RTC.setSQIMode(sqiModeAlarmBoth);
    RTC.setOscillatorStopFlag(false);

    Serial.println("ALARM CLOCKS ARE GO!");

    RTC.setTCXORate( tempScanRate256sec );
    cmdRegisters(NULL);

    RTC.setTCXORate( tempScanRate64sec );
    cmdRegisters(NULL);

    RTC.readAlarm(1, alarmMode, alarmSetting);
    if (
        alarmMode == alarmModeSecondsMatch and
        alarmSetting.Second == 57
        ) {
        Serial.println("Alarm 1 reads correctly");
    } else {
        Serial.println("Alarm 1 reads incorrectly");
    }
    RTC.readAlarm(2, alarmMode, alarmSetting);
    if (
        alarmMode == alarmModePerMinute and
        alarmSetting.Second == 0
        ) {
        Serial.println("Alarm 2 reads correctly");
    } else {
        Serial.println("Alarm 2 reads incorrectly");
    }

    buflen = 0;
    cmdHelp(0);
    RTC.clearAlarmFlag(3);
}

void loop() {
    if (alarm_triggered) showTrigger();

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

void rtcAlarm()  // Triggered when alarm pin is brought low by RTC
{
    alarm_triggered = true;
}

void showTrigger()
{
    dsrtc_calendar_t tm;

    if(alarm_triggered) {
        RTC.read(tm);
        if (RTC.isAlarmFlag(1)) {
            printTime(tm);
            Serial.println(": Alarm 1 triggered");
            RTC.clearAlarmFlag(1);
        }
        if (RTC.isAlarmFlag(2)) {
            printTime(tm);
            Serial.println(": Alarm 2 triggered");
            RTC.clearAlarmFlag(2);
        }
        alarm_triggered = false;
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

void printTime(dsrtc_calendar_t tm)
{
    printDec2(tm.Hour);
    Serial.print(':');
    printDec2(tm.Minute);
    Serial.print(':');
    printDec2(tm.Second);
}

// "TIME" command.
void cmdTime(const char *args)
{
    dsrtc_calendar_t tm;

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
        RTC.writeTime(tm);
        Serial.print("Time has been set to: ");
    }

    // Read the current time.
    RTC.read(tm);
    printTime(tm);
    Serial.println();
}

// "DATE" command.
void cmdDate(const char *args)
{
    dsrtc_calendar_t rtcDate;
    tmElements_t tm, tm2; // Using Time.h to calculate day of week

    if (*args != '\0') {
        // Set the current date.
        unsigned long value = 0;
        tm.Hour = 0;
        tm.Minute = 0;
        tm.Second = 0;
        rtcDate.Second = tm.Second;
        rtcDate.Minute = tm.Minute;
        rtcDate.Hour = tm.Hour;
        while (*args >= '0' && *args <= '9')
            value = value * 10 + (*args++ - '0');
        if (value < 20000000 || value >= 22000000) {
            Serial.println("Year must be between 2000 and 2199");
            return;
        }
        tm.Day = (byte)(value % 100);
        tm.Month = (byte)((value / 100) % 100);
        int year = (value / 10000);
        tm.Year = (byte)((year) - 1970) % 100;
        rtcDate.Day = tm.Day;
        rtcDate.Month = tm.Month;
        rtcDate.Year = year % 100;
        if (tm.Month < 1 || tm.Month > 12) {
            Serial.println("Month must be between 1 and 12");
            return;
        }
        byte len = monthLength(&rtcDate);
        if (tm.Day < 1 || tm.Day > len) {
            Serial.print("Day must be between 1 and ");
            Serial.println(len, DEC);
            return;
        }
        // Set week day
        tm.Year = tm.Year - 1970; // Time.h uses Unix epoch
        breakTime(makeTime(tm), tm2);
        tm.Wday = tm2.Wday;
        rtcDate.Wday = tm.Wday;
        RTC.writeDate(rtcDate);
        Serial.print("Date has been set to: ");
    } /* */

    // Read the current date.
    RTC.read(rtcDate);
    if (rtcDate.Wday > 0 and rtcDate.Wday < 8) Serial.print(days[rtcDate.Wday - 1]);
    Serial.print(rtcDate.Day, DEC);
    Serial.print(months[rtcDate.Month - 1]);
    Serial.println(rtcDate.Year, DEC);
}

// "TEMP" command.
void cmdTemp(const char *args)
{
    tpElements_t tp;
    RTC.readTemperature(tp);
    if (tp.Temp != NO_TEMPERATURE) {
        float temp = tp.Temp + (tp.Decimal / 100);
        Serial.print(temp);
        Serial.println("'C");
    } else {
        Serial.println("Temperature is not available");
    }
}

void printAlarm(byte alarmNum, const alarmMode_t mode, const dsrtc_calendar_t time, bool ison)
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
    dsrtc_calendar_t time;
    alarmMode_t mode;
    bool ison;
    for (byte alarmNum = 1; alarmNum <= 2; ++alarmNum) {
        RTC.readAlarm(alarmNum, mode, time);
        ison = RTC.isAlarmInterrupt(alarmNum);
        printAlarm(alarmNum, mode, time, ison);
    }
    showTrigger();

    if (*args != '\0')
    {
        Serial.println("Clearing alarm flags");
        RTC.clearAlarmFlag(1);
        RTC.clearAlarmFlag(2);
    }
}

const char s_OFF[] PROGMEM = "OFF";

// "ALARM" command.
void cmdAlarm(const char *args)
{
    dsrtc_calendar_t time;
    alarmMode_t mode;
    bool ison;

    memset(&time, 0, sizeof(dsrtc_calendar_t));

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
            bool a1 = RTC.isAlarmInterrupt(1);
            bool a2 = RTC.isAlarmInterrupt(2);
            if (alarmNum == 1) {
              a1 = true;
            } else {
              a2 = true;
            }
            if (a1 && a2) {
              RTC.setSQIMode(sqiModeAlarmBoth);
            } else if (a1) {
              RTC.setSQIMode(sqiModeAlarm1);
            } else if (a2) {
              RTC.setSQIMode(sqiModeAlarm2);
            }
        }
        RTC.writeAlarm(alarmNum, mode, time);
    }

    // Print the current state of the alarm.
    RTC.readAlarm(alarmNum, mode, time);
    ison = RTC.isAlarmInterrupt(alarmNum);
    printAlarm(alarmNum, mode, time, ison);
}

// "REGS" command
void cmdRegisters(const char *)
{
    byte value;
    value = RTC.readControlRegister();
    Serial.write("Ctrl: ");
    print_binary( value );
    value = RTC.readStatusRegister();
    Serial.write("Stat: ");
    print_binary( value );
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
const char s_cmdRegisters[] PROGMEM = "REGS";
const char s_cmdRegistersDesc[] PROGMEM =
    "Show the Control and Status registers";
const char s_cmdHelp[] PROGMEM = "HELP";
const char s_cmdHelpDesc[] PROGMEM =
    "Prints this help message";
const command_t commands[] PROGMEM = {
    {s_cmdTime, cmdTime, s_cmdTimeDesc, s_cmdTimeArgs},
    {s_cmdDate, cmdDate, s_cmdDateDesc, s_cmdDateArgs},
    {s_cmdTemp, cmdTemp, s_cmdTempDesc, 0},
    {s_cmdAlarms, cmdAlarms, s_cmdAlarmsDesc, 0},
    {s_cmdAlarm, cmdAlarm, s_cmdAlarmDesc, s_cmdAlarmArgs},
    {s_cmdRegisters, cmdRegisters, s_cmdRegistersDesc, 0},
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

// -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil -*-
