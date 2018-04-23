/*
 * TinySerialOut.cpp
 *
 * For transmitting debug data over bit bang serial with 38400 baud for a TINY running on 1 MHz
 * 1 Start, 8 Data, 1 Stop, No Parity
 * 26,04 cycles per bit, 260,4 per byte for 38400 baud
 * 8,680 cycles per bit, 86,8 per byte for 115200 baud
 *
 * 34.722 cycles per bit => 350 cycles per byte needed for 230400 baud for a TINY running on 8 MHz
 *
 * Uses PB1 / Pin6 on ATtiny85 as output. To change this modify line 24 in TinySerialOut.h or or set it as Symbol like "-DTX_PIN PB2".
 *
 * if you use the C Version: -> see USE_ASSEMBLER_VERSION
 *      In order to guarantee the correct timing, compile with Arduino standard settings or:
 *      avr-g++ -I"C:\arduino\hardware\arduino\avr\cores\arduino" -I"C:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
 *      Tested with Arduino 1.6.8 and 1.8.5/gcc4.9.2
 *      C Version does not work with AVR gcc7.3.0, since optimization is too bad
 *
 *
 *  Copyright (C) 2015-2018  Armin Joachimsmeyer
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include "TinySerialOut.h"

#include <avr/io.h>         // for PORTB - is also included by <avr/interrupt.h>
#include <avr/pgmspace.h>   // for pgm_read_byte_near()
#include <avr/eeprom.h>     // for eeprom_read_byte()
#include <stdlib.h>         // for utoa() etc.

// use assembler version of smaller code using loops (code size is 76 Byte including first call)
// makes it independent of compiler optimizations
#define USE_ASSEMBLER_VERSION

#ifndef _NOP
#define _NOP()  __asm__ volatile ("nop")
#endif

#ifndef PORTB
#define PORTB (*(volatile uint8_t *)((0x18) + 0x20))
#endif

/*
 * Used for writeStringSkipLeadingSpaces() and therefore all write<type>()
 */
bool sUseCliSeiForStrings = false;
void useCliSeiForStrings(bool aUseCliSeiForStrings) {
    sUseCliSeiForStrings = aUseCliSeiForStrings;
}

#ifndef USE_ASSEMBLER_VERSION
/*
 * Only multiple of 4 cycles are possible. Last loop is only 3 cycles. Setting of a4Microseconds is 2 cycles
 * 3 -> 13 cycles
 * 4 -> 17 cycles
 * 5 -> 21 cycles
 * 6 -> 25 cycles
 */
inline void delay4CyclesInlineExact(uint16_t a4Microseconds) {
    // the following loop takes 4 cycles (4 microseconds  at 1MHz) per iteration + 2 for setting of a4Microseconds
    asm volatile (
            "1: sbiw %0,1" "\n\t"// 2 cycles
            "brne .-4" : "=w" (a4Microseconds) : "0" (a4Microseconds)// 2 cycles
    );
}
#endif

#if (F_CPU == 1000000)
#ifdef USE_115200BAUD
/*
 * 8,680 cycles per bit, 86,8 per byte for 115200 baud
 *
 *  Assembler code for 115200 baud extracted from Digispark core files:
 *  Code size is 196 Byte (including first call)
 *
 *   TinyDebugSerial.h - Tiny write-only software serial.
 *   Copyright 2010 Rowdy Dog Software. This code is part of Arduino-Tiny.
 *
 *   Arduino-Tiny is free software: you can redistribute it and/or modify it
 *   under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 */
void write1Start8Data1StopNoParity(uint8_t aValue) {
    asm volatile
    (
            "cli" "\n\t"

            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- 0 */
            "ror   %[value]" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */

            "brcs  L%=b0h" "\n\t" /* 1  (not taken) */
            "nop" "\n\t" /* 1 */
            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- st is 9 cycles */
            "rjmp  L%=b0z" "\n\t" /* 2 */
            "L%=b0h: " /* 2  (taken) */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- st is 9 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "L%=b0z: "
            "ror   %[value]" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */

            "brcs  L%=b1h" "\n\t" /* 1  (not taken) */
            "nop" "\n\t" /* 1 */
            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b0 is 8 cycles */
            "rjmp  L%=b1z" "\n\t" /* 2 */
            "L%=b1h: " /* 2  (taken) */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b0 is 8 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "L%=b1z: "
            "ror   %[value]" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */

            "brcs  L%=b2h" "\n\t" /* 1  (not taken) */
            "nop" "\n\t" /* 1 */
            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b1 is 9 cycles */
            "rjmp  L%=b2z" "\n\t" /* 2 */
            "L%=b2h: " /* 2  (taken) */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b1 is 9 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "L%=b2z: "
            "ror   %[value]" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */

            "brcs  L%=b3h" "\n\t" /* 1  (not taken) */
            "nop" "\n\t" /* 1 */
            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b2 is 9 cycles */
            "rjmp  L%=b3z" "\n\t" /* 2 */
            "L%=b3h: " /* 2  (taken) */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b2 is 9 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "L%=b3z: "
            "ror   %[value]" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */

            "brcs  L%=b4h" "\n\t" /* 1  (not taken) */
            "nop" "\n\t" /* 1 */
            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b3 is 8 cycles */
            "rjmp  L%=b4z" "\n\t" /* 2 */
            "L%=b4h: " /* 2  (taken) */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b3 is 8 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "L%=b4z: "
            "ror   %[value]" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */

            "brcs  L%=b5h" "\n\t" /* 1  (not taken) */
            "nop" "\n\t" /* 1 */
            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b4 is 9 cycles */
            "rjmp  L%=b5z" "\n\t" /* 2 */
            "L%=b5h: " /* 2  (taken) */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b4 is 9 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "L%=b5z: "
            "ror   %[value]" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */

            "brcs  L%=b6h" "\n\t" /* 1  (not taken) */
            "nop" "\n\t" /* 1 */
            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b5 is 9 cycles */
            "rjmp  L%=b6z" "\n\t" /* 2 */
            "L%=b6h: " /* 2  (taken) */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b5 is 9 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "L%=b6z: "
            "ror   %[value]" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */

            "brcs  L%=b7h" "\n\t" /* 1  (not taken) */
            "nop" "\n\t" /* 1 */
            "cbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b6 is 8 cycles */
            "rjmp  L%=b7z" "\n\t" /* 2 */
            "L%=b7h: " /* 2  (taken) */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b6 is 8 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "L%=b7z: "
            "nop" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */

            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "sbi   %[serreg], %[serbit]" "\n\t" /* 2  <--- b7 is 9 cycles */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            "nop" "\n\t" /* 1 */
            /*    <---sp is 9 cycles */

            "sei" "\n\t"

            :
            :
            [value] "r" ( aValue ),
            [serreg] "I" ( 0x18 ), /* 0x18 is PORTB on Attiny 85 */
            [serbit] "I" ( TX_PIN )
    );
}
#else
/*
 * 26,04 cycles per bit, 260,4 per byte for 38400 baud
 * 17,36 cycles per bit, 173,6 per byte for 57600 baud -> therefore use 38400
 * 24 cycles between each cbi/sbi (Clear/Set Bit in IO-register) command. 2 cycles for each cbi/sbi instruction.
 * code size is 76 Byte (including first call)
 */
void write1Start8Data1StopNoParity(uint8_t aValue) {
#ifdef USE_ASSEMBLER_VERSION
    asm volatile
    (
            "cbi  %[txport] , %[txpin]" "\n\t"    // 2    PORTB &= ~(1 << TX_PIN);
            "nop" "\n\t"// 1    _nop"();
            //delay4CyclesInlineExact(4);
            "ldi  r30 , 0x04" "\n\t"// 1
            "ldi  r31 , 0x00" "\n\t"// 1
            "delay1:"
            "sbiw r30 , 0x01" "\n\t"// 2
            "brne delay1" "\n\t"// 1-2

            "ldi r25 , 0x08" "\n\t"// 1
            "nop" "\n\t"// 1    _nop"();
            "nop" "\n\t"// 1    _nop"();
            // Start of loop
            // if (aValue & 0x01) {
            "loop:"
            "sbrs %[value] , 0" "\n\t"// 1
            "rjmp low" "\n\t"// 2

            "nop" "\n\t"// 1
            "sbi %[txport] , %[txpin]" "\n\t"// 2    PORTB |= 1 << TX_PIN;
            "rjmp shift" "\n\t"// 2

            "low:"
            "cbi %[txport] , %[txpin]" "\n\t"// 2    PORTB &= ~(1 << TX_PIN);
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1

            "shift:"
            "lsr %[value]" "\n\t"// 1    aValue = aValue >> 1;
            // 2 cycles padding
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1
            // delay4CyclesInlineExact(3);
            "ldi  r30 , 0x03" "\n\t"// 1
            "ldi  r31 , 0x00" "\n\t"// 1
            "delay2:"
            "sbiw r30 , 0x01" "\n\t"// 2
            "brne delay2" "\n\t"// 1-2

            // }while (i > 0);
            "subi r25 , 0x01" "\n\t"// 1
            "brne loop" "\n\t"// 1-2
            // To compensate for missing loop cycles at last bit
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1

            // Stop bit
            "sbi %[txport] , %[txpin]" "\n\t"// 2    PORTB |= 1 << TX_PIN;
            // delay4CyclesInlineExact(4)
            "ldi r30 , 0x04" "\n\t"// 1
            "ldi r31 , 0x00" "\n\t"// 1
            "delay3:"
            "sbiw r30 , 0x01" "\n\t"//
            "brne delay3" "\n\t"// 1-2

            :
            :
            [value] "r" ( aValue ),
            [txport] "I" ( 0x18 ) , /* 0x18 is PORTB on Attiny 85 */
            [txpin] "I" ( TX_PIN )
            :
            "r25",
            "r30",
            "r31"
    );
#else
    /*
     * C Version here. You see, it is simple :-)
     */
    // start bit
    PORTB &= ~(1 << TX_PIN);
    _NOP();
    delay4CyclesInlineExact(4);

    // 8 data bits
    uint8_t i = 8;
    do {
        if (aValue & 0x01) {
            // bit=1
            // to compensate for jump at data=0
            _NOP();
            PORTB |= 1 << TX_PIN;
        } else {
            // bit=0
            PORTB &= ~(1 << TX_PIN);
            // compensate for different cycles of sbrs
            _NOP();
            _NOP();
        }
        aValue = aValue >> 1;
        // 3 cycles padding
        _NOP();
        _NOP();
        _NOP();
        delay4CyclesInlineExact(3);
        --i;
    }while (i > 0);

    // to compensate for missing loop cycles at last bit
    _NOP();
    _NOP();
    _NOP();
    _NOP();

    // Stop bit
    PORTB |= 1 << TX_PIN;
    // -8 cycles to compensate for fastest repeated call (1 ret + 1 load + 1 call)
    delay4CyclesInlineExact(4);// gives minimum 25 cycles for stop bit :-)
#endif
}

#endif
#endif

#if (F_CPU == 8000000)
/*
 * 34,72 cycles per bit, 347,2 per byte for 230400 baud at 8MHz Clock
 * 33 cycles between each cbi (Clear Bit in Io-register) or sbi command. 2 cycles for each cbi/sbi instruction.
 */
void write1Start8Data1StopNoParity(uint8_t aValue) {
#ifdef USE_ASSEMBLER_VERSION
    asm volatile
    (
            "\n\t" // Clk
            "cbi  %[txport] , %[txpin]" "\n\t"// 2    PORTB &= ~(1 << TX_PIN);
            "nop" "\n\t"// 1    _nop"();
            "nop" "\n\t"// 1    _nop"();
            //delay4CyclesInlineExact(6);
            "ldi  r30 , 0x06" "\n\t"// 1
            "ldi  r31 , 0x00" "\n\t"// 1
            "delay1:"
            "sbiw r30 , 0x01" "\n\t"// 2
            "brne delay1" "\n\t"// 1-2

            "ldi r25 , 0x08" "\n\t"// 1
            "nop" "\n\t"// 1    _nop"();
            "nop" "\n\t"// 1    _nop"();
            // Start of loop
            // if (aValue & 0x01) {
            "loop:"
            "sbrs %[value] , 0" "\n\t"// 1
            "rjmp .+6" "\n\t"// 2

            "nop" "\n\t"// 1
            "sbi %[txport] , %[txpin]" "\n\t"// 2    PORTB |= 1 << TX_PIN;
            "rjmp .+6" "\n\t"// 2

            "cbi %[txport] , %[txpin]" "\n\t"// 2    PORTB &= ~(1 << TX_PIN);
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1
            "lsr %[value]" "\n\t"// 1    aValue = aValue >> 1;
            // 2 cycles padding
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1
            // delay4CyclesInlineExact(5);
            "ldi r30 , 0x05" "\n\t"// 1
            "ldi r31 , 0x00" "\n\t"// 1
            "delay2:"
            "sbiw r30 , 0x01" "\n\t"// 2
            "brne delay2" "\n\t"// 1-2

            // }while (i > 0);
            "subi r25 , 0x01" "\n\t"// 1
            "brne loop" "\n\t"// 1-2
            // To compensate for missing loop cycles at last bit
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1
            "nop" "\n\t"// 1

            // Stop bit
            "sbi %[txport] , %[txpin]" "\n\t"// 2    PORTB |= 1 << TX_PIN;
            // delay4CyclesInlineExact(4)
            "ldi r30 , 0x06" "\n\t"// 1
            "ldi r31 , 0x00" "\n\t"// 1
            "delay3:"
            "sbiw r30 , 0x01" "\n\t"//
            "brne delay3" "\n\t"// 1-2

            :
            :
            [value] "r" ( aValue ),
            [txport] "I" ( 0x18 ) , /* 0x18 is PORTB on Attiny 85 */
            [txpin] "I" ( TX_PIN )
            :
            "r25",
            "r30",
            "r31"
    );
#else
    /*
     * C Version here. You see, it is simple :-)
     */
    // start bit
    PORTB &= ~(1 << TX_PIN);
    _NOP();
    _NOP();
    delay4CyclesInlineExact(6);

    // 8 data bits
    uint8_t i = 8;
    do {
        if (aChar & 0x01) {
            // bit=1
            // to compensate for jump at data=0
            _NOP();
            PORTB |= 1 << TX_PIN;
        } else {
            // bit=0
            PORTB &= ~(1 << TX_PIN);
            // compensate for different cycles of sbrs
            _NOP();
            _NOP();
        }
        aChar = aChar >> 1;
        delay4CyclesInlineExact(6);
        --i;
    }while (i > 0);

    // to compensate for missing loop cycles at last bit
    _NOP();
    _NOP();
    _NOP();
    _NOP();

    // Stop bit
    PORTB |= 1 << TX_PIN;
    // -8 cycles to compensate for fastest repeated call (1 ret + 1 load + 1 call)
    delay4CyclesInlineExact(6);// gives minimum 33 cycles for stop bit :-)
#endif
}
#endif

/*
 * Write String residing in RAM
 */
void writeString(const char * aStringPtr) {
    if (sUseCliSeiForStrings) {
        while (*aStringPtr != 0) {
            write1Start8Data1StopNoParityWithCliSei(*aStringPtr++);
        }
    } else {
        while (*aStringPtr != 0) {
            write1Start8Data1StopNoParity(*aStringPtr++);
        }
    }
}

/*
 * Write string residing in program space (FLASH)
 */
void writeString_P(const char * aStringPtr) {
    uint8_t tChar = pgm_read_byte_near((const uint8_t * ) aStringPtr);
    // Comparing with 0xFF is safety net for wrong string pointer
    while (tChar != 0 && tChar != 0xFF) {
        if (sUseCliSeiForStrings) {
            write1Start8Data1StopNoParity(tChar);
        } else {
            write1Start8Data1StopNoParityWithCliSei(tChar);
        }
        tChar = pgm_read_byte_near((const uint8_t * ) ++aStringPtr);
    }
}

/*
 * Write string residing in EEPROM space
 */
void writeString_E(const char * aStringPtr) {
    uint8_t tChar = eeprom_read_byte((const uint8_t *) aStringPtr);
    // Comparing with 0xFF is safety net for wrong string pointer
    while (tChar != 0 && tChar != 0xFF) {
        if (sUseCliSeiForStrings) {
            write1Start8Data1StopNoParity(tChar);
        } else {
            write1Start8Data1StopNoParityWithCliSei(tChar);
        }
        tChar = eeprom_read_byte((const uint8_t *) ++aStringPtr);
    }
}

void writeStringWithoutCliSei(const char * aStringPtr) {
    while (*aStringPtr != 0) {
        write1Start8Data1StopNoParity(*aStringPtr++);
    }
}

void writeStringWithCliSei(const char * aStringPtr) {
    while (*aStringPtr != 0) {
        write1Start8Data1StopNoParityWithCliSei(*aStringPtr++);
    }
}

void writeStringSkipLeadingSpaces(const char * aStringPtr) {
    // skip leading spaces
    while (*aStringPtr == ' ' && *aStringPtr != 0) {
        aStringPtr++;
    }
    if (sUseCliSeiForStrings) {
        while (*aStringPtr != 0) {
            write1Start8Data1StopNoParityWithCliSei(*aStringPtr++);
        }
    } else {
        while (*aStringPtr != 0) {
            write1Start8Data1StopNoParity(*aStringPtr++);
        }
    }
}

void writeUnsignedByte(uint8_t aByte) {
    char tStringBuffer[4];
    utoa(aByte, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

/*
 * 2 Byte Hex output with 2 Byte prefix "0x"
 */
void writeUnsignedByteHex(uint8_t aByte) {
    char tStringBuffer[5];
    tStringBuffer[0] = '0';
    tStringBuffer[1] = 'x';
    utoa(aByte, &tStringBuffer[2], 16);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

/*
 * 2 Byte Hex output if it must be short :-)
 */
void writeUnsignedByteHexWithoutPrefix(uint8_t aByte) {
    char tStringBuffer[3];
    utoa(aByte, &tStringBuffer[0], 16);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeByte(int8_t aByte) {
    char tStringBuffer[5];
    itoa(aByte, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeInt(int aInteger) {
    char tStringBuffer[7];
    itoa(aInteger, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeUnsignedInt(unsigned int aInteger) {
    char tStringBuffer[6];
    itoa(aInteger, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeLong(long aLong) {
    char tStringBuffer[12];
    ltoa(aLong, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeFloat(double aFloat) {
    char tStringBuffer[11];
    dtostrf(aFloat, 10, 3, tStringBuffer);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

/*
 * Generated Assembler code for write1Start8Data1StopNoParity() - 1MHz version.
 * Check with .lss file created with: avr-objdump -h -S MyProgram.elf  >"MyProgram.lss".
 *
 c4 98           cbi 0x18, 1
 1 00 00           nop
 2 eb e0           ldi r30, 0x04
 3 f0 e0           ldi r31, 0x00
 4+5 31 97           sbiw    r30, 0x01
 6 + n*4 f1 f7           brne    .-4
 7 98 e0           ldi r25, 0x08
 8 2a e0           ldi r18, 0x03
 9 30 e0           ldi r19, 0x00

 Start of loop
 if (aChar & 0x01) {
 1 80 ff           sbrs    r24, 0
 2+3 02 c0           rjmp    .+6
 PORTB |= 1 << TX_PIN;
 3 00 00           nop
 4+5 c4 9a           sbi 0x18, 1
 6+7 02 c0           rjmp    .+6
 PORTB &= ~(1 << TX_PIN);
 4+5 c4 98           cbi 0x18, 1
 6 00 00           nop
 7 00 00           nop

 8 86 95           lsr r24
 9 00 00           nop
 10 00 00           nop
 11 00 00           nop
 12 f9 01           movw    r30, r18
 13+14 31 97           sbiw    r30, 0x01
 15 f1 f7           brne    .-4
 16 91 50           subi    r25, 0x01
 17+18 a1 f7           brne    .-34

 To compensate for missing loop cycles at last bit
 00 00           nop
 00 00           nop
 00 00           nop
 00 00           nop

 Stop bit
 c4 9a           sbi 0x18, 1
 8a e0           ldi r24, 0x04
 90 e0           ldi r25, 0x00
 01 97           sbiw    r24, 0x01
 f1 f7           brne    .-4

 08 95           ret

 *
 */
