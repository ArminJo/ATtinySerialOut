# Attiny Serial Out
Minimal bit-bang send serial

38400 or 115200 baud for 1 Mhz or 230400 baud for 8 MHz Attiny clock
### Perfect for debugging purposes
### Code size only 76 Bytes@38400/230400Bd or 196 Bytes@115200Bd (including first call)
### Uses PB1 / Pin6 on ATtiny85 as output.
To change the output pin, just modify line 24 in TinySerialOut.h. You will find the file in the Arduino IDE under "Sketch/Show Sketch Folder" (or Ctrl+K) and then in the TinySerialOut/src directory. Or set it as Symbol like "-DTX_PIN PB2".

## Download
 The actual version can be downloaded directly from GitHub [here] (https://github.com/ArminJo/ATtinySerialOut/blob/master/TinySerialOut.zip?raw=true)

C versions of code included for better understanding and to enable own modifications.
In order to guarantee the correct timing, compile C version with Arduino standard settings or:
```
avr-g++ -I"C:\arduino\hardware\arduino\avr\cores\arduino" -I"C:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
```
