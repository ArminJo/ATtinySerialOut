/*
 * TinySerialOutExample.ino
 * Example für using Serial Out library
 */

#include <Arduino.h> // needed for main()

#include <avr/pgmspace.h> // needed for PSTR()
#include "TinySerialOut.h"

#define VERSION "1.0"

#if (F_CPU != 1000000) &&  (F_CPU != 8000000)
#error "F_CPU value must be 1000000 or 8000000."
#endif

void setup(void) {
    initTXPin();

    writeString("START\nVersion ");
    writeString( VERSION);
    writeString(" from  ");
    writeString(__DATE__);
    write1Start8Data1StopNoParity('\n');

    uint8_t tOSCCAL = OSCCAL;
    /*
     * Example of using writeString_P()
     */
    writeString_P(PSTR("Value of OSCCAL is:"));
    writeUnsignedByteHex(tOSCCAL);
}

void loop(void) {
    static uint8_t tIndex = 0;
    /*
     * Example of 3 Byte output. View in combined ASSCI / HEX View in HTerm (http://www.der-hammer.info/terminal/)
     * Otherwise use writeUnsignedByteHexWithoutPrefix or writeUnsignedByteHex
     */
    write1Start8Data1StopNoParityWithCliSei('I');
    writeUnsignedByte(tIndex); // 1 Byte output
    //writeUnsignedByteHexWithoutPrefix(tIndex); // 2 Byte output
    //writeUnsignedByteHex(tIndex); // 4 Byte output
    write1Start8Data1StopNoParityWithCliSei('\n');
}
