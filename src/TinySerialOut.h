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

#ifndef TINY_SERIAL_OUT_H_
#define TINY_SERIAL_OUT_H_

#include <stdint.h>
#include <avr/interrupt.h>  // for cli() and sei()
#include <avr/io.h>

/*
 * Change this, if you need another pin as serial output
 * or set it as Symbol like "-DTX_PIN PB2"
 */
#ifndef TX_PIN
#define TX_PIN     PB1 // (Pin6 on Tiny85) - use one of PB0 to PB4 (+PB5) here
#endif

#define USE_115200BAUD // use bigger (+120 bytes for unrolled loop) but faster code

extern bool sUseCliSeiForStrings;
void useCliSeiForStrings(bool aUseCliSeiForStrings);

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

void writeString(const char * aStringPtr);
void writeString_P(const char * aStringPtr);
void writeString_E(const char * aStringPtr);
void writeStringWithCliSei(const char * aStringPtr);
void writeStringWithoutCliSei(const char * aStringPtr);
void writeStringSkipLeadingSpaces(const char * aStringPtr);

void writeByte(int8_t aByte);
void writeUnsignedByte(uint8_t aByte);
void writeUnsignedByteHex(uint8_t aByte);
void writeUnsignedByteHexWithoutPrefix(uint8_t aByte);
void writeInt(int aInteger);
void writeUnsignedInt(unsigned int aInteger);
void writeLong(long aLong);
void writeFloat(double aFloat);

#endif /* TINY_SERIAL_OUT_H_ */
