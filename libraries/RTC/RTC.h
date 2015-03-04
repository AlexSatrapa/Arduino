#ifndef __rtc_common_h__
#define __rtc_common_h__

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

typedef struct  { 
  int8_t Temp; 
  uint8_t Decimal; 
} tpElements_t, TempElements, *tpElementsPtr_t;

// Bits in the Control register
// Based on page 13 of specs; http://www.maxim-ic.com/datasheet/index.mvp/id/4984
#define DS3232_EOSC         0x80
#define DS3232_BBSQW        0x40
#define DS3232_CONV         0x20
#define DS3232_RS2          0x10
#define DS3232_RS1          0x08
#define DS3232_INTCN        0x04
#define DS3232_A2IE         0x02
#define DS3232_A1IE         0x01

#define DS3232_RS_1HZ       0x00
#define DS3232_RS_1024HZ    0x08
#define DS3232_RS_4096HZ    0x10
#define DS3232_RS_8192HZ    0x18

// Bits in the Status register
// Based on page 14 of specs; http://www.maxim-ic.com/datasheet/index.mvp/id/4984
#define DS3232_OSF          0x80
#define DS3232_BB33KHZ      0x40
#define DS3232_CRATE1       0x20
#define DS3232_CRATE0       0x10
#define DS3232_EN33KHZ      0x08
#define DS3232_BSY          0x04
#define DS3232_A2F          0x02
#define DS3232_A1F          0x01

#define DS3232_CRATE_64     0x00
#define DS3232_CRATE_128    0x10
#define DS3232_CRATE_256    0x20
#define DS3232_CRATE_512    0x30

#endif
