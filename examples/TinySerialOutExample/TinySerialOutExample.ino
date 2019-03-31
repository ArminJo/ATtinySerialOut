/*
 * TinySerialOutExample.ino
 * Example für using Serial Out library
 */

#include <Arduino.h> // needed for main()

#include <avr/pgmspace.h> // needed for PSTR()
#include "TinySerialOut.h"

#define VERSION_EXAMPLE "1.0"

void setup(void) {
    initTXPin();

    writeString(F("START " __FILE__ "\nVersion " VERSION_EXAMPLE " from " __DATE__ "\n"));

    uint8_t tOSCCAL = OSCCAL;

    writeString("Value of OSCCAL is:");
    writeUnsignedByteHexWithPrefix(tOSCCAL);
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
