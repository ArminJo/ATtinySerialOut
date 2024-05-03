<div align = center>

# [Attiny Serial Out](https://github.com/ArminJo/ATtinySerialOut)
Minimal bit-bang send serial. 115200 baud for 1/8/16 MHz ATtiny clock.<br/>

[![Badge License: GPLv3](https://img.shields.io/badge/License-GPLv3-brightgreen.svg)](https://www.gnu.org/licenses/gpl-3.0)
 &nbsp; &nbsp; 
[![Badge Version](https://img.shields.io/github/v/release/ArminJo/ATtinySerialOut?include_prereleases&color=yellow&logo=DocuSign&logoColor=white)](https://github.com/ArminJo/ATtinySerialOut/releases/latest)
 &nbsp; &nbsp; 
[![Badge Commits since latest](https://img.shields.io/github/commits-since/ArminJo/ATtinySerialOut/latest?color=yellow)](https://github.com/ArminJo/ATtinySerialOut/commits/master)
 &nbsp; &nbsp; 
[![Badge Build Status](https://github.com/ArminJo/ATtinySerialOut/workflows/LibraryBuild/badge.svg)](https://github.com/ArminJo/ATtinySerialOut/actions)
 &nbsp; &nbsp; 
![Badge Hit Counter](https://visitor-badge.laobi.icu/badge?page_id=ArminJo_ATtinySerialOut)
<br/>
<br/>
[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)

Available as [Arduino library "ATtinySerialOut"](https://www.arduinolibraries.info/libraries/a-ttiny-serial-out).

[![Button Install](https://img.shields.io/badge/Install-brightgreen?logoColor=white&logo=GitBook)](https://www.ardu-badge.com/ATtinySerialOut)
 &nbsp; &nbsp; 
[![Button Changelog](https://img.shields.io/badge/Changelog-blue?logoColor=white&logo=AzureArtifacts)](https://github.com/ArminJo/ATtinySerialOut?tab=readme-ov-file#revision-history)

</div>

#### If you find this library useful, please give it a star.

&#x1F30E; [Google Translate](https://translate.google.com/translate?sl=en&u=https://github.com/ArminJo/ATtinySerialOut)

<br/>

# Features
- Perfect for debugging purposes.
- Provides Serial.print / println functions for easy software porting.
- Code size is only 76 bytes@38400 baud or 196 bytes@115200 baud (including first call).
- Provides additional fast printHex() and printlnHex() functions.
- Default TX pin is PIN_PB2 on an ATtiny85.

<br/>

## Serial functions provided (linefeed is \n instead of \r\n):
```c++
    void print(const __FlashStringHelper *aStringPtr);
    void print(const char *aStringPtr);
    void print(char aChar);
    void print(uint8_t aByte, uint8_t aBase = 10);
    void print(int16_t, uint8_t aBase = 10);
    void print(uint16_t aInteger, uint8_t aBase = 10);
    void print(uint32_t aLong, uint8_t aBase = 10);
    void print(uint32_t aLong, uint8_t aBase = 10);
    void print(double aFloat, uint8_t aDigits = 2);

    void printHex(uint8_t aByte); // with 0x prefix

    void println(const __FlashStringHelper *aStringPtr);
    void println(char aChar);
    void println(uint8_t aByte, uint8_t aBase = 10);
    void println(int16_t aInteger, uint8_t aBase = 10);
    void println(uint16_t aInteger, uint8_t aBase = 10);
    void println(int32_t aLong, uint8_t aBase = 10);
    void println(uint32_t aLong, uint8_t aBase = 10);
    void println(double aFloat, uint8_t aDigits = 2);

    void println(void);
```
<br/>

# Example
```
#include <Arduino.h>

#if defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#define TX_PIN PIN_PA1 // (package pin 2 / TXD on Tiny167) - can use one of PIN_PA0 to PIN_PA7 here
#else
#define TX_PIN PIN_PB2 // (package pin 7 on Tiny85) - can use one of PIN_PB0 to PIN_PB4 (+PIN_PB5) here
#endif

void setup(void) {
    initTXPin();
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_ATTINY_SERIAL_OUT));
    writeString("OSCCAL=");
    writeUnsignedByteHexWithPrefix(OSCCAL);
}
void loop() {
}
```

# Using the new *.hpp files
In order to support [compile options](#compile-options--macros-for-this-library) more easily,
the line `#include <ATtinySerialOut.h>` must be changed to  `#include <ATtinySerialOut.hpp>`
in your main program (aka *.ino file with setup() and loop()).

In **all other files** you must use `#include <ATtinySerialOut.h>`, to **prevent `multiple definitions` linker errors**:

If you forget to include *ATtinySerialOut.hpp*, you will see errors like `ATtinySerialOutExample.cpp:38: undefined reference to `initTXPin()`.

<br/>

# Compile options / macros for this library
To customize the library to different requirements, there are some compile options / macros available.<br/>
These macros must be defined in your program **before** the line `#include <TinySerialOut.hpp>` to take effect.<br/>
Modify them by enabling / disabling them, or change the values if applicable.

| Name | Default value | Description |
|-|-:|-|
| `TX_PIN` | PIN_PB2 (PIN_PA1 for ATtiny87/167) | The pin to use for transmitting bit bang serial. These pin names are valid for ATTinyCore and may be different in other cores. |
| `TINY_SERIAL_DO_NOT_USE_115200BAUD` | disabled | To force using other baud rates. The rates are **38400 baud at 1 MHz** (which has smaller code size) or **230400 baud at 8/16 MHz**. |
| `TINY_SERIAL_INHERIT_FROM_PRINT` | disabled | If defined, you can use this class as a replacement for standard Serial as a print class e.g.  for functions like void `prinInfo(Print *aSerial)`. Increases program size. |

<br/>

# [OpenWindowAlarm example](https://raw.githubusercontent.com/ArminJo/ATtinySerialOut/master/examples/OpenWindowAlarm/OpenWindowAlarm.ino)
This example issues an alarm if the chip sensor detect a falling teperarure and is fully documented [here](https://github.com/ArminJo/Arduino-OpenWindowAlarm)

<br/>

# Troubleshooting
### Error `call of overloaded 'println(fstr_t*)' is ambiguous`
Please use the [new Digistump core](https://github.com/ArminJo/DigistumpArduino#installation).
Since version 1.2.0, the library is no longer compatible with the old cores supplied by digistump.

### Error `type 'TinySerialOut' is not a base type for type 'TwoWire' using Print::write;`
Reason: Another class, e.g. ATTinyCore `class TwoWire : public Stream` uses the Print class or a Print method, but Print is normally redefined by ATtinySerialOut.
You must define TINY_SERIAL_INHERIT_FROM_PRINT before including ATtinySerialOut.hpp, to avoid this error.

<br/>

# Revision History
### Version 2.3.0
- Support of all ports of ATtiny88 using ATTinyCore numbering scheme.

### Version 2.2.1
- Usage of ATTinyCore pin numbering also for ATtiny167.

### Version 2.2.0
- Usage of ATTinyCore pin numbering scheme e.g. PIN_PB2 and therefore removed `USE_PORTB_FOR_TX_PIN`.

### Version 2.1.1
- Adjusted macro for MH-ET LIVE Core.

### Version 2.1.0
- Added compile guard.
- Added `USE_PORTB_FOR_TX_PIN` to allow TX pin on port B for ATtiny87/167.
- Improved #if guard for instance naming.

### Version 2.0.0 - 09/2021
- Renamed ATtinySerialOut.cpp to ATtinySerialOut.hpp => TX pin can be defined in main program.
- Added symbol `TINY_SERIAL_INHERIT_FROM_PRINT` - if defined, you can use this class as a replacement for standard Serial as a print class.

### Version 1.2.2 - 03/2021
- Cloned ATtinySerialOut.cpp as ATtinySerialOut.hpp for direct include.
- Added ATtiny88 support.

### Version 1.2.1 - 10/2020
- Added ATtinyX4 support.

### Version 1.2.0 - 7/2020
- Removed workaround `#define __FlashStringHelper fstr_t` for old Digispark core < 1.6.8 in order to work with core >= 1.7.0.
- Added ATtiny167 support.
- Moved functions from h to cpp file.

### Version 1.1.0 - 5/2020
- Removed symbol `TINY_SERIAL_INHERIT_FROM_PRINT` and replaced by macro `#define Print TinySerialOut`
- Changed `int` to `int16_t` and `long` to `int32_t`.
- Use `utoa()` and `ultoa()` for `uint16_t` and `uint32_t`.

### Version 1.0.5 - 3/2020
- Added function `writeCRLF()`

### Version 1.0.4 - 1/2020
- Symbol TINY_SERIAL_INHERIT_FROM_PRINT - if defined, you can use this class as a replacement for standard Serial as a print class.
- Improved all ...Hex() functions.
- Define FlashStringHelper for digispark.
- Changed label `loop` to `txloop` in `write1Start8Data1StopNoParity()` for 38400 baud.

### Version 1.0.3
- OpenWindowAlarm example updated.

### Version 1.0.2
- `printHex()` now using capital letters.
- Improved OpenWindowAlarm example.

### Version 1.0.1
- Renamed example to be consistent.

### Version 1.0.0
Initial Arduino library version.

### Remark
C version of serial code is included for better understanding, but assembler version is used. This is because for the C code the timing depends on compiler optimisation switches. You should get the right timing if you compile the C code it with Arduino standard settings or:
```
avr-g++ -I"C:\arduino\hardware\arduino\avr\cores\arduino" -I"C:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
```