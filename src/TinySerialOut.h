/*
 * TinySerialOut.h
 *
 *  Copyright (C) 2015-2018  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *  License: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 *  This file is part of TinySerialOut https://github.com/ArminJo/ATtinySerialOut.
 *
 */

//
// ATMEL ATTINY85
//
//                                         +-\/-+
//        PCINT5/!RESET/ADC0/dW (D5) PB5  1|    |8  Vcc
// PCINT3/XTAL1/CLKI/!OC1B/ADC3 (D3) PB3  2|    |7  PB2 (D2) SCK/USCK/SCL/ADC1/T0/INT0/PCINT2
//  PCINT4/XTAL2/CLKO/OC1B/ADC2 (D4) PB4  3|    |6  PB1 (D1) MISO/DO/AIN1/OC0B/OC1A/PCINT1 / TX Debug output
//                                   GND  4|    |5  PB0 (D0) MOSI/DI/SDA/AIN0/OC0A/!OC1A/AREF/PCINT0
//                                         +----+
#ifndef TINY_SERIAL_OUT_H_
#define TINY_SERIAL_OUT_H_

#include <stdint.h>
#include <stddef.h> // for size_t
#include <avr/interrupt.h>  // for cli() and sei()
#include <avr/io.h>

#if (F_CPU != 1000000) &&  (F_CPU != 8000000) &&  (F_CPU != 16000000)
#error "F_CPU value must be 1000000, 8000000 or 16000000."
#endif

/*
 * Change this, if you need another pin as serial output
 * or set it as Symbol like "-DTX_PIN PB1"
 */
#ifndef TX_PIN
#define TX_PIN  PB2 // (case pin 7 on Tiny85) - use one of PB0 to PB4 (+PB5) here
#endif

/*
 * @1MHz use bigger (+120 bytes for unrolled loop) but faster code. Otherwise only 38400 baud is possible.
 * @8/16 MHz use 115200 baud instead of 230400 baud.
 */
#ifndef TINY_SERIAL_DO_NOT_USE_115200BAUD  // define this to force using other baud rates
#define USE_115200BAUD
#endif

/*
 * Define or uncomment this, if you want to save code size and if you can live with 87 micro seconds intervals of disabled interrupts for each sent byte.
 */
//#define USE_ALWAYS_CLI_SEI_GUARD_FOR_OUTPUT
extern bool sUseCliSeiForWrite; // default is true
void useCliSeiForStrings(bool aUseCliSeiForWrite);

inline void initTXPin() {
    // TX_PIN is active LOW, so set it to HIGH initially
    PORTB |= (1 << TX_PIN);
    // set pin direction to output
    DDRB |= (1 << TX_PIN);
}

void write1Start8Data1StopNoParity(uint8_t aValue);
inline void write1Start8Data1StopNoParityWithCliSei(uint8_t aValue) {
    cli();
    write1Start8Data1StopNoParity(aValue);
    sei();
}

inline void writeValue(uint8_t aValue) {
    write1Start8Data1StopNoParity(aValue);
}


// The same class as for plain arduino
#ifndef F
class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))
#endif

void writeString(const char * aStringPtr);
void writeString(const __FlashStringHelper * aStringPtr);
void writeString_P(const char * aStringPtr);
void writeString_E(const char * aStringPtr);
void writeStringWithCliSei(const char * aStringPtr);
void writeStringWithoutCliSei(const char * aStringPtr);
void writeStringSkipLeadingSpaces(const char * aStringPtr);

void writeBinary(uint8_t aByte); // write direct without decoding
void writeChar(uint8_t aChar); // Synonym for writeBinary
void writeByte(int8_t aByte);
void writeUnsignedByte(uint8_t aByte);
void writeUnsignedByteHex(uint8_t aByte);
void writeUnsignedByteHexWithPrefix(uint8_t aByte);
void writeInt(int aInteger);
void writeUnsignedInt(unsigned int aInteger);
void writeLong(long aLong);
void writeUnsignedLong(unsigned long aLong);
void writeFloat(double aFloat);
void writeFloat(double aFloat, uint8_t aDigits);

class TinyDebugSerial {
public:
    void print(const __FlashStringHelper * aStringPtr);
    void print(const char* aStringPtr);
    void print(char aChar);
    void print(uint8_t aByte, uint8_t aBase = 10);
    void print(int aInteger, uint8_t aBase = 10);
    void print(unsigned int aInteger, uint8_t aBase = 10);
    void print(long aLong, uint8_t aBase = 10);
    void print(unsigned long aLong, uint8_t aBase = 10);
    void print(double aFloat, uint8_t aDigits = 2);

    void printHex(uint8_t aByte); // with 0x prefix

    void println(const __FlashStringHelper * aStringPtr);
    void println(char aChar);
    void println(uint8_t aByte, uint8_t aBase = 10);
    void println(int aInteger, uint8_t aBase = 10);
    void println(unsigned int aInteger, uint8_t aBase = 10);
    void println(long aLong, uint8_t aBase = 10);
    void println(unsigned long aLong, uint8_t aBase = 10);
    void println(double aFloat, uint8_t aDigits = 2);

    void println(void);
};

extern TinyDebugSerial Serial;

#endif /* TINY_SERIAL_OUT_H_ */
