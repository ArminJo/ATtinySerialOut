# Attiny Serial Out

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Minimal bit-bang send serial

115200 baud for 1/8/16 MHz ATtiny clock.
### Perfect for debugging purposes.
### Provides Serial.print / println functions for easy software porting. 
### Code size only 76 Bytes including first call, except for 115200 baud at 1 Mhz, there we need 196 Bytes.
### Uses PB2 / Pin7 on ATtiny85 as output.
To change the output pin, just modify line 38 in TinySerialOut.h. You will find the file in the Arduino IDE under "Sketch/Show Sketch Folder" (or Ctrl+K) and then in the libraries/TinySerialOut/src directory. Or set it as compiler symbol like "-DTX_PIN PB1".

## Download
 The actual version can be downloaded directly from GitHub [here](https://github.com/ArminJo/ATtinySerialOut/blob/master/extras/TinySerialOut.zip?raw=true)

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

C versions of code included for better understanding.
In order to guarantee the correct timing with the C Version, compile it with Arduino standard settings or:
```
avr-g++ -I"C:\arduino\hardware\arduino\avr\cores\arduino" -I"C:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
```
