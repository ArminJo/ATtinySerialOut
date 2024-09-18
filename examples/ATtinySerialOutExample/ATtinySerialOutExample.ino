/*
 * ATtinySerialOutExample.cpp
 * Example for using ATtinySerialOut library
 *
 *  Copyright (C) 2015-2021  Armin Joachimsmeyer
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

#include <Arduino.h> // required for main()

/*
 * You can specify TX_PIN here (before the line #include "ATtinySerialOut.hpp")
 */
//#if defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
//#define TX_PIN PIN_PA1 // (package pin 2 / TXD on Tiny167) - can use one of PIN_PA0 to PIN_PA7 here
//#elif defined(__AVR_ATtiny84__)
//#define TX_PIN PIN_PA1 // can use one of PIN_PA0 to PIN_PA7 and PIN_PB0 to PIN_PB2 here
//#else
//#define TX_PIN PIN_PB2 // (package pin 7 on Tiny85) - can use one of PIN_PB0 to PIN_PB4 (+PIN_PB5) here
//#endif
#if defined(__AVR_ATtiny13__) || defined(__AVR_ATtiny13A__)
#define TINY_SERIAL_DO_NOT_USE_115200BAUD // This saves 120 bytes but sends with only 38400 baud.
#endif

#include "ATtinySerialOut.hpp"

void setup(void) {
    initTXPin();
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_ATTINY_SERIAL_OUT));

    writeString("OSCCAL=");
#if !defined(__AVR_ATtiny13__) && !defined(__AVR_ATtiny13A__)
    writeUnsignedByteHexWithPrefix(OSCCAL);
#endif
}

void loop(void) {
    static uint8_t tIndex = 0;
    /*
     * Example of 3 byte output. View in combined ASSCI / HEX View in HTerm (http://www.der-hammer.info/terminal/)
     * Otherwise use writeUnsignedByteHexWithoutPrefix or writeUnsignedByteHex
     */
    write1Start8Data1StopNoParityWithCliSei('I');
    writeBinary(tIndex);                    // 1 byte binary output

#if !defined(__AVR_ATtiny13__) && !defined(__AVR_ATtiny13A__) // the utoa() function used in writeUnsignedByte etc. requires too much program space!
    writeUnsignedByte(tIndex);              // 1-3 byte ASCII output
    writeUnsignedByteHexWithPrefix(tIndex); // 4 byte output
    writeUnsignedByteHex(tIndex);           // 2 byte output
    write1Start8Data1StopNoParityWithCliSei('\n');

    /*
     * Serial.print usage example
     */
    Serial.print("I=");
    Serial.print((char) tIndex);
    Serial.print(" | ");
    Serial.print(tIndex);
    Serial.print(" | ");
    Serial.print(tIndex, HEX);
    Serial.print(" | ");
    Serial.printHex(tIndex);
    Serial.print(" | ");
    Serial.println(tIndex);
#endif
    tIndex++;
    delay(100);
}
