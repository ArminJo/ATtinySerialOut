/*
 * ATtinySerialOut.h
 *
 *  Copyright (C) 2015-2023  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *
 *  This file is part of TinySerialOut https://github.com/ArminJo/ATtinySerialOut.
 *
 *  TinySerialOut is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

//
// ATMEL ATTINY85
//
//                                               +-\/-+
//               PCINT5/!RESET/ADC0/dW (5) PB5  1|    |8  VCC
// USB+   PCINT3/XTAL1/CLKI/!OC1B/ADC3 (3) PB3  2|    |7  PB2 (2) SCK/USCK/SCL/ADC1/T0/INT0/PCINT2 / TX Debug output
// USB-   PCINT4/XTAL2/CLKO/ OC1B/ADC2 (4) PB4  3|    |6  PB1 (1) MISO/DO/AIN1/OC0B/ OC1A/PCINT1 - (Digispark) LED
//                                         GND  4|    |5  PB0 (0) MOSI/DI/AIN0/OC0A/!OC1A/SDA/AREF/PCINT0
//                                               +----+
// ATMEL ATTINY167
// Pin numbers are for Digispark core
// Pin numbers in parenthesis are for ATTinyCore
//
//                    +-\/-+
//    RX  6 (0) PA0  1|    |20  PB0 (D8)  0 OC1AU  TONE  Timer 1 Channel A
//    TX  7 (1) PA1  2|    |19  PB1 (9)  1 OC1BU  Internal LED
//        8 (2) PA2  3|    |18  PB2 (10) 2 OC1AV  Timer 1 Channel B
//   INT1 9 (3) PA3  4|    |17  PB3 (11) 4 OC1BV  connected with 51 ohm to D- and 3.3 volt zener diode.
//             AVCC  5|    |16  GND
//             AGND  6|    |15  VCC
//       10 (4) PA4  7|    |14  PB4 (12) XTAL1
//       11 (5) PA5  8|    |13  PB5 (13) XTAL2
//       12 (6) PA6  9|    |12  PB6 (14) 3 INT0  connected with 68 ohm to D+ (and disconnected 3.3 volt zener diode). Is terminated with ~20 kOhm if USB attached :-(
//        5 (7) PA7 10|    |11  PB7 (15) RESET
//                    +----+
//
// MH-ET LIVE Tiny88 (16.0MHz) board
// Digital Pin numbers in parenthesis are for ATTinyCore library
//                        USB
//                      +-\__/-+
//               PA2  15|      |14  PB7
//               PA3  16|      |13  PB5 SCK
//           D17 PA0  A6|      |12  PB4 MISO
//           D18 PA1  A7|      |11  PB3 MOSI
//     (D17) D19 PC0  A0|      |10  PB2 OC1B/PWM SS
//     (D18) D20 PC1  A1|      |9   PB1 OC1A/PWM
//     (D19) D21 PC2  A2|      |8   PB0
//     (D20) D22 PC3  A3|      |7   PD7 RX
//SDA  (D21) D23 PC4  A4|      |6   PD6 TX
//SCL  (D22) D24 PC5  A5|      |5   PD5
//     (D23) D25 PC7  25|      |4   PD4
//RESET          PC6 RST|      |3   PD3 INT1
//LED            PD0   0|      |5V
//USB+           PD1   1|      |GND
//USB-  INT0     PD2   2|      |VIN
//                      +------+
#ifndef _ATTINY_SERIAL_OUT_H
#define _ATTINY_SERIAL_OUT_H

#if defined(__AVR_ATtiny13__) || defined(__AVR_ATtiny13A__) || defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) \
    || defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) \
    || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__) \
    || defined(__AVR_ATtiny88__)
#include <Arduino.h>

#define VERSION_ATTINY_SERIAL_OUT "2.2.0"
#define VERSION_ATTINY_SERIAL_OUT_MAJOR 2
#define VERSION_ATTINY_SERIAL_OUT_MINOR 2
#define VERSION_ATTINY_SERIAL_OUT_PATCH 0
// The change log is at the bottom of the file

/*
 * Macro to convert 3 version parts into an integer
 * To be used in preprocessor comparisons, such as #if VERSION_ATTINY_SERIAL_OUT_HEX >= VERSION_HEX_VALUE(3, 0, 0)
 */
#define VERSION_HEX_VALUE(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define VERSION_ATTINY_SERIAL_OUT_HEX  VERSION_HEX_VALUE(VERSION_ATTINY_SERIAL_OUT_MAJOR, VERSION_ATTINY_SERIAL_OUT_MINOR, VERSION_ATTINY_SERIAL_OUT_PATCH)

#if (F_CPU != 1000000) &&  (F_CPU != 8000000) &&  (F_CPU != 16000000)
#error F_CPU value must be 1000000, 8000000 or 16000000.
#endif

#if !defined(TX_PIN)
#  if defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__) // Digispark PRO board
#    if defined PIN_PA1
// ATTinyCore
#define TX_PIN  PIN_PA1 // (package pin 2 / TXD on Tiny167) - can use one of PA0 to PA7 here
#    elif defined PA1   //
#define TX_PIN  PA1     // (package pin 2 / TXD on Tiny167) - can use one of PA0 to PA7 here
#    endif

#  elif defined(__AVR_ATtiny88__) //  MH-ET LIVE Tiny88(16.0MHz) board
#define TX_PIN  PIN_PD6 // (board pin 6) - can use one of PD3 to PD7 here

#  elif defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
#define TX_PIN  PIN_PB2 // Can use one of PIN_PA0 to PIN_PA7 and PIN_PB0 to PIN_PB2 here

#  else// Digispark board
#    if defined(DIGISTUMPCORE)
#define TX_PIN PB2 // (package pin 7 on Tiny85) - can use one of PB0 to PB4 (+PB5) here
#    else
// ATTinyCore
#define TX_PIN PIN_PB2 // (package pin 7 on Tiny85) - can use one of PB0 to PB4 (+PB5) here
#    endif
#  endif
#endif

/*
 * Activate this, if you want to save 10 bytes code size and if you can live
 * with 87 micro seconds intervals of disabled interrupts for each sent byte @115200 baud.
 */
//#define USE_ALWAYS_CLI_SEI_GUARD_FOR_OUTPUT
/*
 * @1 MHz use bigger (+120 bytes for unrolled loop) but faster code. Otherwise only 38400 baud is possible.
 * @8/16 MHz use 115200 baud instead of 230400 baud.
 */
//#define TINY_SERIAL_DO_NOT_USE_115200BAUD
#if !defined(TINY_SERIAL_DO_NOT_USE_115200BAUD)  // define this to force using other baud rates
#define _USE_115200BAUD // to avoid double negations
#endif

/*
 * If defined, you can use this class as a replacement for standard Serial as a print class e.g.
 * for functions where you require a Print class like in void prinInfo(Print *aSerial). Increases program size.
 */
//#define TINY_SERIAL_INHERIT_FROM_PRINT

// The same class definition as for plain arduino
#if not defined(F)
class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))
#endif

extern bool sUseCliSeiForWrite; // default is true
void useCliSeiForStrings(bool aUseCliSeiForWrite); // might be useful to set to false if output is done from ISR, to avoid to call unwanted sei().

void initTXPin(); // Must be called once if pin is not set to output otherwise
void write1Start8Data1StopNoParity(uint8_t aValue);
void write1Start8Data1StopNoParityWithCliSei(uint8_t aValue);
void writeValue(uint8_t aValue);

void writeString(const char *aStringPtr);
void writeString(const __FlashStringHelper *aStringPtr);
void writeString_P(const char *aStringPtr);
void writeString_E(const char *aStringPtr);
void writeStringWithCliSei(const char *aStringPtr);
void writeStringWithoutCliSei(const char *aStringPtr);
void writeStringSkipLeadingSpaces(const char *aStringPtr);

void writeBinary(uint8_t aByte); // write direct without decoding
void writeChar(uint8_t aChar); // Synonym for writeBinary
void writeCRLF();
void writeByte(int8_t aByte);
void writeUnsignedByte(uint8_t aByte);
void writeUnsignedByteHex(uint8_t aByte);
void writeUnsignedByteHexWithPrefix(uint8_t aByte);
void writeInt(int16_t aInteger);
void writeUnsignedInt(uint16_t aInteger);
void writeLong(int32_t aLong);
void writeUnsignedLong(uint32_t aLong);
void writeFloat(double aFloat);
void writeFloat(double aFloat, uint8_t aDigits);

char nibbleToHex(uint8_t aByte);

#if defined(TINY_SERIAL_INHERIT_FROM_PRINT)
class TinySerialOut: public Print
#else
class TinySerialOut
#endif
{
public:

    void begin(long);
    void end();
    void flush(void); // not required -> dummy

    void printHex(uint8_t aByte); // with 0x prefix
    void printHex(uint16_t aWord); // with 0x prefix
    void printlnHex(uint8_t aByte); // with 0x prefix
    void printlnHex(uint16_t aWord); // with 0x prefix

    // virtual functions of Print class
    size_t write(uint8_t aByte);
    operator bool(); // To support "while (!Serial); // wait for serial port to connect. Required for Leonardo only

#if !defined(TINY_SERIAL_INHERIT_FROM_PRINT)
    void print(const __FlashStringHelper *aStringPtr);
    void print(const char *aStringPtr);
    void print(char aChar);
    void print(uint8_t aByte, uint8_t aBase = 10);
    void print(int16_t aInteger, uint8_t aBase = 10);
    void print(uint16_t aInteger, uint8_t aBase = 10);
    void print(int32_t aLong, uint8_t aBase = 10);
    void print(uint32_t aLong, uint8_t aBase = 10);
    void print(double aFloat, uint8_t aDigits = 2);

    void println(const char *aStringPtr);
    void println(const __FlashStringHelper *aStringPtr);
    void println(char aChar);
    void println(uint8_t aByte, uint8_t aBase = 10);
    void println(int16_t aInteger, uint8_t aBase = 10);
    void println(uint16_t aInteger, uint8_t aBase = 10);
    void println(int32_t aLong, uint8_t aBase = 10);
    void println(uint32_t aLong, uint8_t aBase = 10);
    void println(double aFloat, uint8_t aDigits = 2);

    void println(void);
#endif // TINY_SERIAL_INHERIT_FROM_PRINT

};

// This if is required to be compatible with ATTinyCores and AttinyDigisparkCores
#if defined(USE_SOFTWARE_SERIAL) && (7-USE_SOFTWARE_SERIAL-7 == 14)
#define USE_SOFTWARE_SERIAL 1   // define it to 1 if it is only defined, but has no value
#endif
#if defined(DEFAULT_TO_TINY_DEBUG_SERIAL) /*AttinyDigisparkCore condition for defining Serial at line 745 in TinyDebugSerial.h*/ \
    || ((!defined(UBRRH) && !defined(UBRR0H)) || (defined(USE_SOFTWARE_SERIAL) && USE_SOFTWARE_SERIAL)) /*ATTinyCore condition for defining Serial at line 55 in TinySoftwareSerial.h*/\
    || ((defined(UBRRH) || defined(UBRR0H) || (defined(LINBRRH)) && !USE_SOFTWARE_SERIAL)) /*ATTinyCore condition for for defining Serial at line 71ff in HardwareSerial.h*/
extern TinySerialOut SerialOut; // Name our instance SerialOut since Serial is already declared
#define Serial SerialOut // Redirect all usages of Serial to our SerialOut instance :-)
#else
// No known cores here -> delete possible existent definitions of Serial, before we declare our object (below)
#  if defined(Serial)
#undef Serial
#  endif
extern TinySerialOut Serial; // if there is no Serial object, we can name the instance of our class Serial :-)
#endif

#if !defined(TINY_SERIAL_INHERIT_FROM_PRINT)
#define Print TinySerialOut
#endif

#endif // defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)

#endif // _ATTINY_SERIAL_OUT_H
