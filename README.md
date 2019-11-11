# Attiny Serial Out

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Installation instructions](https://www.ardu-badge.com/badge/ATtinySerialOut.svg?)](https://www.ardu-badge.com/ATtinySerialOut)
[![Commits since latest](https://img.shields.io/github/commits-since/ArminJo/ATtinySerialOut/latest)](https://github.com/ArminJo/ATtinySerialOut/commits/master)
[![Build Status](https://travis-ci.org/ArminJo/ATtinySerialOut.svg?branch=master)](https://travis-ci.org/ArminJo/ATtinySerialOut)
[![Hit Counter](https://hitcounter.pythonanywhere.com/count/tag.svg?url=https%3A%2F%2Fgithub.com%2FArminJo%2FATtinySerialOut)](https://github.com/brentvollebregt/hit-counter)

Minimal bit-bang send serial

115200 baud for 1/8/16 MHz ATtiny clock.
### Perfect for debugging purposes.
### Provides Serial.print / println functions for easy software porting. 
### Code size is only 76 Bytes@38400 baud or 196 Bytes@115200 baud (including first call)
### Default TX pin is PB2 on a ATtiny85.
To change the output pin, just modify line 38 in TinySerialOut.h. You will find the file in the Arduino IDE under "Sketch/Show Sketch Folder" (or Ctrl+K) and then in the libraries/TinySerialOut/src directory. Or set it as compiler symbol like "-DTX_PIN PB1".

## Serial functions provided (linefeed is \n instead of \r\n):
```   
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
```

### Remark
C version of serial code is included for better understanding, but assembler version is used. This is because for the C code the timing depends on compiler optimisation switches. You should get the right timing if you compile the C code it with Arduino standard settings or:
```
avr-g++ -I"C:\arduino\hardware\arduino\avr\cores\arduino" -I"C:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
```
