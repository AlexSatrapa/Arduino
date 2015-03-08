#ifndef __rtc_common_h__
#define __rtc_common_h__

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <Time.h>

enum alarmMode_t {
  alarmModeUnknown,       // not in spec table
  alarmModePerSecond,     // once per second, A1 only
  alarmModePerMinute,     // once per minute, A2 only
  alarmModeSecondsMatch,  // when seconds match, A1 only
  alarmModeMinutesMatch,  // when minutes [and seconds] match
  alarmModeHoursMatch,    // when hours, minutes [and seconds] match
  alarmModeDateMatch,     // when date (of month), hours, minutes [and seconds] match
  alarmModeDayMatch,      // when day (of week), hours, minutes [and seconds] match
  alarmModeOff            // set to date or day, but value is 0
  };

enum sqiMode_t {
  sqiModeNone, 
  sqiMode1Hz, 
  sqiMode1024Hz, 
  sqiMode4096Hz, 
  sqiMode8192Hz, 
  sqiModeAlarm1, 
  sqiModeAlarm2, 
  sqiModeAlarmBoth
  };

enum tempScanRate_t {
  tempScanRate64sec,
  tempScanRate128sec,
  tempScanRate256sec,
  tempScanRate512sec
  };

typedef struct {
  int8_t Temp; 
  uint8_t Decimal; 
} tpElements_t, TempElements, *tpElementsPtr_t;

// Bits in the Control register
// Based on page 13 of specs; http://www.maxim-ic.com/datasheet/index.mvp/id/4984
#define DS323X_EOSC         0x80
#define DS323X_BBSQW        0x40
#define DS323X_CONV         0x20
#define DS323X_RS2          0x10
#define DS323X_RS1          0x08
#define DS323X_INTCN        0x04
#define DS323X_A2IE         0x02
#define DS323X_A1IE         0x01

#define DS323X_RS_1HZ       0x00
#define DS323X_RS_1024HZ    0x08
#define DS323X_RS_4096HZ    0x10
#define DS323X_RS_8192HZ    0x18

// Bits in the Status register
// Based on page 14 of specs; http://www.maxim-ic.com/datasheet/index.mvp/id/4984
#define DS323X_OSF          0x80
#define DS323X_BB33KHZ      0x40
#define DS323X_CRATE1       0x20
#define DS323X_CRATE0       0x10
#define DS323X_EN33KHZ      0x08
#define DS323X_BSY          0x04
#define DS323X_A2F          0x02
#define DS323X_A1F          0x01

#define DS323X_CRATE_64     0x00
#define DS323X_CRATE_128    0x10
#define DS323X_CRATE_256    0x20
#define DS323X_CRATE_512    0x30

#define NO_TEMPERATURE      0x7F

#endif
