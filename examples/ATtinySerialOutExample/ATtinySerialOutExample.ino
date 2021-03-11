/*
 * ATtinySerialOutExample.cpp
 * Example for using ATtinySerialOut library
 *
 *  Copyright (C) 2015-2019  Armin Joachimsmeyer
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

#include <Arduino.h> // required for main()

/*
 * You can specify TX_PIN here, but then you must remove ATtinySerialOut.cpp from the library,
 * otherwise you will see "multiple definition of" errors.
 */
//#if defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
//#define TX_PIN PA1 // (package pin 2 / TXD on Tiny167) - can use one of PA0 to PA7 here
//#else
//#define TX_PIN PB2 // (package pin 7 on Tiny85) - can use one of PB0 to PB4 (+PB5) here
//#endif
//#include "ATtinySerialOut.cpp.h" // activate this to enable TX_PIN settings

#include "ATtinySerialOut.h"

void setup(void) {
    initTXPin();

    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_ATTINY_SERIAL_OUT));

    writeString("OSCCAL=");
    writeUnsignedByteHexWithPrefix(OSCCAL);
}

void loop(void) {
    static uint8_t tIndex = 0;
    /*
     * Example of 3 Byte output. View in combined ASSCI / HEX View in HTerm (http://www.der-hammer.info/terminal/)
     * Otherwise use writeUnsignedByteHexWithoutPrefix or writeUnsignedByteHex
     */
    write1Start8Data1StopNoParityWithCliSei('I');
    writeBinary(tIndex);                    // 1 Byte binary output
    writeUnsignedByte(tIndex);              // 1-3 Byte ASCII output
    writeUnsignedByteHexWithPrefix(tIndex); // 4 Byte output
    writeUnsignedByteHex(tIndex);           // 2 Byte output
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

    tIndex++;
    delay(100);
}
