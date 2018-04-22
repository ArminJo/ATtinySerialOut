# Attiny Serial Out
Minimal bit-bang send serial 38400 or 115200 baud for 1 Mhz or 230400 baud for 8 MHz Attiny clock
### Perfect for debugging purposes
### Code size only 76 Bytes@38400Bd or 196 Bytes@115200Bd (including first call)

## Download
 The actual version can be downloaded directly from GitHub [here] (https://github.com/ArminJo/NeoPatterns/blob/master/TinySerialOut.zip?raw=true)

C versions of code included for better understanding and to enable own modifications.
In order to guarantee the correct timing, compile C versions with Arduino standard settings or:
```
avr-g++ -I"C:\arduino\hardware\arduino\avr\cores\arduino" -I"C:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
```
