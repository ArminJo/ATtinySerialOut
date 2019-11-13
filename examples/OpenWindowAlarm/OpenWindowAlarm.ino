/*
 * OpenWindowAlarm.cpp
 *
 * Overview:
 * Every 24 seconds a sample is taken of the ATtiny internal temperature sensor which has a resolution of 1 degree.
 * If temperature is lower than "old" temperature value, then an alarm is issued 5 minutes later, if "the condition still holds".
 * Detection of an open window is indicated by a longer 20 ms blink and a short click every 24 seconds.
 * A low battery (below 3.55 volt for LiPo) is indicated by beeping and flashing LED every 24 seconds. Only the beep (not the flash) is significantly longer than at open window detection.
 *
 * Detailed description:
 * Open window is detected after `TEMPERATURE_COMPARE_AMOUNT * TEMPERATURE_SAMPLE_SECONDS` (48) seconds of reading a temperature
 * which value is `TEMPERATURE_DELTA_THRESHOLD_DEGREE` (2) lower than the temperature `TEMPERATURE_COMPARE_DISTANCE * TEMPERATURE_SAMPLE_SECONDS` (192 -> 3 minutes and 12 seconds) seconds before.
 * The delay is implemented by 3 times sleeping at `SLEEP_MODE_PWR_DOWN` for a period of 8 seconds to reduce power consumption.
 * Detection of an open window is indicated by a longer 20 ms blink and a short click every 24 seconds.
 * Therefore, the internal sensor has 3 minutes time to adjust to the outer temperature, to get even small changes in temperature.
 * The greater the temperature change the earlier the sensor value will change and detect an open window.
 * After open window detection Alarm is activated after `OPEN_WINDOW_ALARM_DELAY_MINUTES` (5).
 *   The alarm will not sound the current temperature is greater than the minimum measured temperature (+ 1) i.e. the window has been closed already.
 *
 * At startup, the battery voltage is measured and recognized if the module is operating on one LIPO battery or two standard AA / AAA batteries.
 * Every `VCC_MONITORING_DELAY_MIN` (60) minutes the battery voltage is measured. A battery voltage below `VCC_VOLTAGE_LOWER_LIMIT_MILLIVOLT_LIPO` (3550) Millivolt
 * or below `VCC_VOLTAGE_LOWER_LIMIT_MILLIVOLT_STANDARD` (2350) mV is indicated by beeping and flashing LED every 24 seconds. Only the beep (not the flash) is significantly longer than at open window detection.
 *
 * The initial alarm lasts for 10 minutes. After this, it is activated for a period of 10 seconds with a increasing break from 24 seconds up to 5 minutes.
 * Check temperature at each end of break interval to discover closed window, if window was closed during the silent break, but device was not reset.
 *
 * After power up or reset, the inactive settling time is 5 minutes or additionally 4:15 (or 8:30) minutes if the board is getting colder during the settling time, to avoid false alarms after boot.
 *
 * Power consumption:
 * Power consumption is 6uA at sleep and 2.8 mA at at 1 MHz active.
 * Loop needs 2.1 ms and with DEBUG 6.5 ms => active time is ca. 1/10k or 1/4k of total time and power consumption is 500 times more than sleep.
 *   => Loop adds 5% to 12% to total power consumption.
 *
 *  Copyright (C) 2018-19  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-OpenWindowAlarm https://github.com/ArminJo/Arduino-OpenWindowAlarm.
 *
 *  Arduino-OpenWindowAlarm is free software: you can redistribute it and/or modify
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

#include <Arduino.h>

/*
 * To see serial output, you must select "Digispark (1mhz – No USB)" as Board in the Arduino IDE!
 * And you need TinySerialOut.h + TinySerialOut.cpp in your sketch folder.
 */
//#define DEBUG // To see serial output with 115200 baud at P2 -
//#define TRACE // To see more serial output at startup with 115200 baud at P2
#define ALARM_TEST // start alarm immediately if PB0 / P0 is connected to ground

#ifdef TRACE
#define DEBUG
#endif
#ifdef DEBUG
#include "ATtinySerialOut.h"
#endif

#include <avr/boot.h>  // needed for boot_signature_byte_get()
#include <avr/power.h> // needed for clock_prescale_set()
#include <avr/sleep.h> // needed for sleep_enable()
#include <avr/wdt.h>   // needed for WDTO_8S

#define VERSION "1.2.2"
/*
 * Version 1.2.2
 * - Converted to Serial.print.
 * - New PWMTone() without tone().
 * Version 1.2.1
 * - Fixed bug in check for temperature rising after each alarm.
 * Version 1.2
 * - Improved sleep, detecting closed window also after start of alarm, reset behavior.
 * - Changed LIPO detection threshold.
 * - Fixed analog reference bug.
 */

#ifdef ALARM_TEST
#define ALARM_TEST_PIN PB0
#endif

const uint8_t OPEN_WINDOW_ALARM_DELAY_MINUTES = 5; // Wait time between window open detection and activation of alarm
const int OPEN_WINDOW_ALARM_FREQUENCY_HIGH = 2200; // Should be the resonance frequency of speaker/buzzer
const int OPEN_WINDOW_ALARM_FREQUENCY_LOW = 1100;
const int OPEN_WINDOW_ALARM_FREQUENCY_VCC_TOO_LOW = 1600; // Use a different frequency to distinguish the this alert from others

/*
 * Temperature timing
 */
const uint16_t TEMPERATURE_SAMPLE_SECONDS = 24;  // Use multiple of 8 here
const uint8_t OPEN_WINDOW_SAMPLES = (OPEN_WINDOW_ALARM_DELAY_MINUTES * 60) / TEMPERATURE_SAMPLE_SECONDS;
const uint8_t TEMPERATURE_COMPARE_AMOUNT = 2;
const uint8_t TEMPERATURE_COMPARE_DISTANCE = 8; // 3 minutes and 12 seconds
// Array to hold enough values to compare TEMPERATURE_COMPARE_AMOUNT values with the same amount of values TEMPERATURE_COMPARE_DISTANCE positions before
uint16_t sTemperatureArray[(TEMPERATURE_COMPARE_AMOUNT + TEMPERATURE_COMPARE_DISTANCE + TEMPERATURE_COMPARE_AMOUNT)];

/*
 * Temperature values
 */
const uint16_t TEMPERATURE_DELTA_THRESHOLD_DEGREE = 2; // 1 LSB  = 1 Degree Celsius
uint16_t sTemperatureNewSum = 0;
uint16_t sTemperatureOldSum = 0;

uint16_t sTemperatureMinimumAfterWindowOpen; // for window close detection
uint16_t sTemperatureAtWindowOpen;

/*
 * Detection flags
 */
bool sOpenWindowDetected = false;
bool sOpenWindowDetectedOld = false;
uint8_t sOpenWindowSampleDelayCounter;

/*
 * VCC monitoring
 */
const uint16_t VCC_VOLTAGE_LOWER_LIMIT_MILLIVOLT_LIPO = 3550; // 3.7 volt is the normal operating voltage if powered by a LiPo battery
const uint16_t VCC_VOLTAGE_LIPO_DETECTION = 3600; // Above 3.6 volt we assume that a LIPO battery is attached, below we assume a CR2032 or two AA or AAA batteries are attached.
const uint16_t VCC_VOLTAGE_LOWER_LIMIT_MILLIVOLT_STANDARD = 2350; // 3.0 volt is normal operating voltage if powered by a CR2032 or two AA or AAA batteries.

uint16_t sVCCVoltageMillivolt;
bool sVCCVoltageTooLow;
bool sLIPOSupplyDetected;
const uint8_t VCC_MONITORING_DELAY_MIN = 60; // Check VCC every hour, because this costs extra power.
uint16_t sVCCMonitoringDelayCounter; // Counter for VCC monitoring.

//
// ATMEL ATTINY85
//
//                                +-\/-+
//          RESET/ADC0 (D5) PB5  1|    |8  Vcc
// Tone      ADC3 USB+ (D3) PB3  2|    |7  PB2 (D2) INT0/ADC1 - TX Debug output
// Tone inv. ADC2 USB- (D4) PB4  3|    |6  PB1 (D1) MISO/DO/AIN1/OC0B/OC1A/PCINT1 - (Digispark) LED
//                          GND  4|    |5  PB0 (D0) OC0A/AIN0 - Alarm Test if connected to ground
//                                +----+

#define LED_PIN  PB1
#define TONE_PIN PB4
#define TONE_PIN_INVERTED PB3

#define ADC_TEMPERATURE_CHANNEL_MUX 15
#define ADC_1_1_VOLT_CHANNEL_MUX 12
// 0 1 0 Internal 1.1 volt voltage Reference.
#define INTERNAL (2)
#define INTERNAL1V1 INTERNAL
#define SHIFT_VALUE_FOR_REFERENCE REFS0

#if (LED_PIN == TX_PIN)
#error "LED pin must not be equal TX pin."
#endif

#define LED_PULSE_LENGTH 200 // 500 is well visible, 200 is OK
#if (LED_PULSE_LENGTH < 150)
#error "LED_PULSE_LENGTH must at least be 150, since the code after digitalWrite(LED_PIN, 1) needs 150 us."
#endif

uint8_t sMCUSRStored; // content of MCUSR register at startup

void PWMtone(unsigned int aFrequency, unsigned int aDurationMillis = 0);
void delayAndSignalOpenWindowDetectionAndLowVCC();
void alarm();
void playDoubleClick();
void readTempAndManageHistory();
void resetHistory();
void initPeriodicSleepWithWatchdog(uint8_t tSleepMode, uint8_t aWatchdogPrescaler);
void sleepDelay(uint16_t aSecondsToSleep);
void delayMilliseconds(unsigned int aMillis);
uint16_t readADCChannelWithReferenceOversample(uint8_t aChannelNumber, uint8_t aReference, uint8_t aOversampleExponent);
uint16_t getVCCVoltageMillivolt(void);
void checkVCCPeriodically();
void changeDigisparkClock();

/***********************************************************************************
 * Code starts here
 ***********************************************************************************/

void setup() {

    /*
     * store MCUSR early for later use
     */
    sMCUSRStored = MCUSR;
    MCUSR = 0; // to prepare for next reset or power on

#ifdef DEBUG
    /*
     * Initialize the serial pin as an output for Serial.print like debugging
     */
    initTXPin();
#endif

    /*
     * initialize the pins
     */
    pinMode(LED_PIN, OUTPUT);
    pinMode(TONE_PIN_INVERTED, OUTPUT);
    pinMode(TONE_PIN, OUTPUT);
#ifdef ALARM_TEST
    pinMode(ALARM_TEST_PIN, INPUT_PULLUP);
#endif

    changeDigisparkClock();

#ifdef DEBUG
    Serial.print(F("START " __FILE__ "\nVersion " VERSION " from " __DATE__ "\nAlarm delay = "));
    Serial.print(OPEN_WINDOW_ALARM_DELAY_MINUTES);
    Serial.println(F(" minutes"));
#endif

#ifdef TRACE
    Serial.print(F("MCUSR=0x"));
    Serial.print(sMCUSRStored);
    Serial.print(F(" LFuse=0x"));
    Serial.print(boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS));
    Serial.print(F(" WDTCR=0x"));
    Serial.print(WDTCR);
    Serial.print(F(" OSCCAL=0x"));
    Serial.println(OSCCAL);
#endif

    /*
     * init sleep mode and wakeup period
     */
    initPeriodicSleepWithWatchdog(SLEEP_MODE_PWR_DOWN, WDTO_8S);
// disable Arduino delay() and millis() timer0 and also its interrupts which kills the deep sleep.
    TCCR0B = 0; // No clock selected
    TIMSK = 0;

    /*
     * Initialize ADC channel and reference
     */
    readADCChannelWithReferenceOversample(ADC_TEMPERATURE_CHANNEL_MUX, INTERNAL1V1, 0);

    /*
     * Signal power on with a single tone or signal reset with a double click.
     */
#ifdef DEBUG
    Serial.print(F("Booting from "));
#endif
    if (sMCUSRStored & (1 << PORF)) {
        PWMtone(OPEN_WINDOW_ALARM_FREQUENCY_HIGH, 100);
#ifdef DEBUG
        Serial.println(F("power up"));
#endif
    } else {
        playDoubleClick();
#ifdef DEBUG
        Serial.println(F("reset"));
#endif
    }

    /*
     * Blink LED at startup to show OPEN_WINDOW_MINUTES
     */
    for (int i = 0; i < OPEN_WINDOW_ALARM_DELAY_MINUTES; ++i) {
        // activate LED
        digitalWrite(LED_PIN, 1);
        delayMilliseconds(200);  // delay() is disabled, so use delayMicroseconds()
        // deactivate LED
        digitalWrite(LED_PIN, 0);
        delayMilliseconds(200);
    }

#ifdef ALARM_TEST
    if (!digitalRead(ALARM_TEST_PIN)) {
#ifdef DEBUG
        Serial.println(F("Test signal out"));
#endif
        alarm();
    }
#endif

    /*
     * Check VCC and decide if LIPO or 2 standard batteries / 1 button cell attached
     */
    sVCCVoltageMillivolt = getVCCVoltageMillivolt();
    if (sVCCVoltageMillivolt > VCC_VOLTAGE_LIPO_DETECTION) {
        sLIPOSupplyDetected = true;
    } else {
        sLIPOSupplyDetected = false;
    }

    sVCCMonitoringDelayCounter = 1; // 1 -> check directly now
    checkVCCPeriodically();

// disable digital input buffer to save power
// do not disable buffer for outputs whose values are read back
    DIDR0 = (1 << ADC1D) | (1 << ADC2D) | (1 << ADC3D) | (1 << AIN1D) | (1 << AIN0D);

    /*
     * wait 8 seconds, since ATtinys temperature is increased after the micronucleus boot process
     */
    sleep_cpu()
    ;
}

/*
 * Shift temperature history values ad insert new value.
 * Check if temperature decreases after power on.
 * Check if window was just opened.
 * If window was opened check if window still open -> ALARM
 * Loop needs 2.1 ms and with DEBUG 6.5 ms => active time is ca. 1/10k or 1/4k of total time and power consumption is 500 times more than sleep.
 * 2 ms for Temperature reading
 * 0.25 ms for processing
 * 0.05 ms for LED flashing
 *  + 4.4 ms for DEBUG
 */
void loop() {

    readTempAndManageHistory(); // needs 2 milliseconds

    // activate LED after reading to signal it. Do it here to reduce delay below.
    digitalWrite(LED_PIN, 1);

    /*
     * Check if we are just after boot and temperature is decreasing
     */
    if ((sTemperatureArray[(sizeof(sTemperatureArray) / sizeof(sTemperatureArray[0])) - 1] == 0)
            && (sTemperatureArray[(sizeof(sTemperatureArray) / sizeof(sTemperatureArray[0])) - 2] > 0)
            /*
             * array is almost full, so check if temperature is lower than at boot time which means,
             * we ported the sensor from a warm place to its final one
             * or the window is still open and the user has pushed the reset button to avoid an alarm.
             */
            && (sTemperatureArray[0] < sTemperatureArray[(sizeof(sTemperatureArray) / sizeof(sTemperatureArray[0])) - 2])) {
        // Start from beginning, clear temperature array
#ifdef DEBUG
        Serial.println(F("Detected porting to a colder place -> reset"));
#endif
        resetHistory();
    } else {

        if (!sOpenWindowDetected) {
            /*
             * Check if window just opened
             */
            // tTemperatureOldSum can be 0 -> do not use tTemperatureNewSum < tTemperatureOldSum - (TEMPERATURE_DELTA_THRESHOLD_DEGREE * TEMPERATURE_COMPARE_AMOUNT)
            if (sTemperatureNewSum + (TEMPERATURE_DELTA_THRESHOLD_DEGREE * TEMPERATURE_COMPARE_AMOUNT) < sTemperatureOldSum) {
#ifdef DEBUG
                Serial.print(F("Detected window just opened -> check again in "));
                Serial.print(OPEN_WINDOW_ALARM_DELAY_MINUTES);
                Serial.println(F(" minutes"));
#endif
                sTemperatureMinimumAfterWindowOpen = sTemperatureNewSum;
                sTemperatureAtWindowOpen = sTemperatureNewSum;
                sOpenWindowDetected = true;
                sOpenWindowSampleDelayCounter = 0;
            }
        } else {
            /*
             * Here open window is detected
             * First check if window already closed -> start a new detection
             */
            if (sTemperatureNewSum > (sTemperatureMinimumAfterWindowOpen + TEMPERATURE_COMPARE_AMOUNT)) {
                sOpenWindowDetected = false;
#ifdef DEBUG
                Serial.println(F("Detected window already closed -> start again"));
#endif
                // reset history in order to avoid a new detection at next sample, since tTemperatureNewSum may still be lower than tTemperatureOldSum
                resetHistory();
            } else {
                if (sTemperatureNewSum < sTemperatureMinimumAfterWindowOpen) {
                    // set new minimum temperature
                    sTemperatureMinimumAfterWindowOpen = sTemperatureNewSum;
                }

                /*
                 * Check if alarm delay was reached
                 */
                sOpenWindowSampleDelayCounter++;
                if (sOpenWindowSampleDelayCounter >= OPEN_WINDOW_SAMPLES) {
                    /*
                     * Here delay is reached
                     * Check if still open - current temperature must be 1 degree lower than temperature at time of open detection
                     * "- TEMPERATURE_COMPARE_AMOUNT": this reduces the sensibility, but helps to detect already closed windows.
                     * You may remove this to increase sensibility.
                     */
                    if (sTemperatureNewSum <= sTemperatureAtWindowOpen - TEMPERATURE_COMPARE_AMOUNT) {
                        /*
                         * Window is still open -> ALARM
                         */
#ifdef DEBUG
                        Serial.println(F("Detected window still open -> alarm"));
#endif
                        alarm();
                    } else {
                        // Temperature not 1 degree lower than temperature at time of open detection
                        sOpenWindowDetected = false;
#ifdef DEBUG
                        Serial.println(F("Assume wrong window open detection -> start again"));
#endif
                    }
                } // delay
            } // already closed
        } // !sOpenWindowDetected
    }  // after power on and temperature is decreasing

    /*
     * VCC check every hour
     */
    checkVCCPeriodically(); // needs 4.5 ms

    delayAndSignalOpenWindowDetectionAndLowVCC(); // Introduce a delay of 22 ms if open window is detected to let the LED light longer
    // deactivate LED before sleeping
    digitalWrite(LED_PIN, 0);

    sleepDelay(TEMPERATURE_SAMPLE_SECONDS);
}

/*
 * Code to change Digispark Bootloader clock settings to get the right CPU frequency
 * and to reset Digispark OCCAL tweak.
 * Call it if you want to use the standard ATtiny85 library, BUT do not call it, if you need Digispark USB functions available for 16 MHz.
 */
void changeDigisparkClock() {
    uint8_t tLowFuse = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    if ((tLowFuse & 0x0F) == 0x01) {
        /*
         * Here we have High Frequency PLL Clock (16 or 16.5 MHz)
         */
#if (F_CPU == 1000000)
        // Divide 16 MHz Pll clock by 16 for Digispark Boards to get the requested 1 MHz
        clock_prescale_set(clock_div_16);
//        CLKPR = (1 << CLKPCE);  // unlock function
//        CLKPR = (1 << CLKPS2); // %16
#endif
#if (F_CPU == 8000000)
        // Divide 16 MHz Pll clock by 2 for Digispark Boards to get the requested 8 MHz
        clock_prescale_set(clock_div_2);
//        CLKPR = (1 << CLKPCE);  // unlock function
//        CLKPR = (1 << CLKPS0);// %2
#endif
    }

    /*
     * Code to reset Digispark OCCAL tweak
     */
#define  SIGRD  5 // needed for boot_signature_byte_get()
    uint8_t tStoredOSCCAL = boot_signature_byte_get(1);
    if (OSCCAL != tStoredOSCCAL) {
#ifdef DEBUG
        uint8_t tOSCCAL = OSCCAL;
        Serial.print(F("Changed OSCCAL from 0x"));
        Serial.print(tOSCCAL);
        Serial.print(F(" to 0x"));
        Serial.println(tStoredOSCCAL);
#endif
        // retrieve the factory-stored oscillator calibration bytes to revert the digispark OSCCAL tweak
        OSCCAL = tStoredOSCCAL;
    }
}

/*
 * Like tone(), but use OC1B (PB4) and (inverted) !OC1B (PB3)
 */
void PWMtone(unsigned int aFrequency, unsigned int aDurationMillis) {

    // Determine which prescaler to use, we are running with 1 MHz now
    uint32_t tOCR = 1000000L / aFrequency;
    uint8_t tPrescaler = 0x01;
    while (tOCR > 0xff && tPrescaler < 15) {
        tPrescaler++;
        tOCR >>= 1;

    }
    OCR1C = tOCR - 1;
    OCR1B = OCR1C / 2; // set PWM to 50%
    GTCCR = (1 << PWM1B) | (1 << COM1B0); // Switch to PWM Mode with OC1B (PB4) + !OC1B (PB3) outputs enabled
    TCCR1 = (tPrescaler << CS10);

    delayMilliseconds(aDurationMillis);
    TCCR1 = 0; // Select no clock
    GTCCR = 0; // Disconnect OC1B + !OC1B
    digitalWrite(TONE_PIN_INVERTED, LOW);
    digitalWrite(TONE_PIN, LOW);
}

/*
 * plays alarm signal for the specified seconds
 */
void playAlarmSignalSeconds(uint16_t aSecondsToPlay) {
#ifdef DEBUG
    Serial.print(F("Play alarm for "));
    Serial.print(aSecondsToPlay);
    Serial.println(F(" seconds"));
#endif
    uint16_t tCounter = (aSecondsToPlay * 10) / 13; // == ... * 1000 (ms per second) / (1300 ms for a loop)
    if (tCounter == 0) {
        tCounter = 1;
    }
    while (tCounter-- != 0) {
        // activate LED
        digitalWrite(LED_PIN, 1);
        PWMtone(OPEN_WINDOW_ALARM_FREQUENCY_LOW, 300);

        // deactivate LED
        digitalWrite(LED_PIN, 0);
        PWMtone(OPEN_WINDOW_ALARM_FREQUENCY_HIGH, 1000);
    }
}

void resetHistory() {
    for (uint8_t i = 0; i < (sizeof(sTemperatureArray) / sizeof(sTemperatureArray[0])) - 1; ++i) {
        sTemperatureArray[i] = 0;
    }
}

void readTempAndManageHistory() {
    sTemperatureNewSum = 0;
    sTemperatureOldSum = 0;
    uint8_t tIndex = (sizeof(sTemperatureArray) / sizeof(sTemperatureArray[0])) - 1;
    /*
     * shift values in temperature history array and insert new one at [0]
     */
    while (tIndex >= TEMPERATURE_COMPARE_AMOUNT + TEMPERATURE_COMPARE_DISTANCE) {
        // shift TEMPERATURE_COMPARE_AMOUNT values to end and sum them up
        sTemperatureArray[tIndex] = sTemperatureArray[tIndex - 1];
        sTemperatureOldSum += sTemperatureArray[tIndex - 1];
        tIndex--;
    }
    while (tIndex >= TEMPERATURE_COMPARE_AMOUNT) {
        // shift values to end
        sTemperatureArray[tIndex] = sTemperatureArray[tIndex - 1];
        tIndex--;
    }
    while (tIndex > 0) {
        // shift (TEMPERATURE_COMPARE_AMOUNT - 1) values to end and sum them up
        sTemperatureArray[tIndex] = sTemperatureArray[tIndex - 1];
        sTemperatureNewSum += sTemperatureArray[tIndex - 1];
        tIndex--;
    }
    /*
     * Read new Temperature 16 times (typical 280 - 320 at 25 C) and add to sum
     * needs 2 ms
     */
    sTemperatureArray[0] = readADCChannelWithReferenceOversample(ADC_TEMPERATURE_CHANNEL_MUX, INTERNAL1V1, 4);
    sTemperatureNewSum += sTemperatureArray[0];

#ifdef DEBUG
    // needs 4.4 ms
    Serial.print(F("Temp="));
    Serial.print(sTemperatureArray[0]);
    Serial.print(F(" Old="));
    Serial.print(sTemperatureOldSum);
    Serial.print(F(" New="));
    Serial.println(sTemperatureNewSum);
#endif
}

/*
 * Check if history is completely filled and if temperature is rising
 */
bool checkForTemperatureRising() {
    if (sTemperatureArray[(sizeof(sTemperatureArray) / sizeof(sTemperatureArray[0])) - 1] != 0
            && sTemperatureNewSum > sTemperatureOldSum + (TEMPERATURE_DELTA_THRESHOLD_DEGREE * TEMPERATURE_COMPARE_AMOUNT)) {
#ifdef DEBUG
        Serial.println(F("Alarm - detected window already closed -> start again"));
#endif
        sOpenWindowDetected = false;
        resetHistory();
        return true;
    }
    return false;
}

/*
 * Generates a 2200 | 1100 Hertz tone signal for 600 seconds / 10 minutes and then play it 10 seconds with intervals starting from 24 seconds up to 5 minutes.
 * After 2 minutes the temperature is checked for the remaining 8 minutes if temperature is increasing in order to detect a closed window.
 * Check temperature at each end of break interval to discover closed window, if window was closed during the silent break, but device was not reset.
 */
void alarm() {

    // First 120 seconds - just generate alarm tone
    playAlarmSignalSeconds(120);
    // after 80 seconds the new (increased) temperature is stable

    // prepare for new temperature check - reset history
    resetHistory();

    // remaining 480 seconds - check temperature while generating alarm tone
    for (uint8_t i = 0; i < 16; ++i) {
        readTempAndManageHistory();
        /*
         * Check if history is completely filled and if temperature is rising
         */
        if (checkForTemperatureRising()) {
            return;
        }
        playAlarmSignalSeconds(30);
    }

#ifdef DEBUG
    Serial.println(F("After 10 minutes alarm now play it for 10 s with 24 to 600 s delay"));
#endif

    uint16_t tDelay = 24;
    /*
     * initialize history with current temperature to detect increasing values
     */
    resetHistory();
    for (uint8_t i = 0; i < TEMPERATURE_COMPARE_DISTANCE; ++i) {
        readTempAndManageHistory();
    }
    while (true) {
#ifdef DEBUG
        Serial.print(F("Alarm pause for "));
        Serial.print(tDelay);
        Serial.println(F(" seconds"));
#endif
        sleepDelay(tDelay); // Start with 24 seconds
        /*
         * check if temperature is rising at end of break interval
         */
        readTempAndManageHistory();
        if (sTemperatureNewSum >= sTemperatureAtWindowOpen + TEMPERATURE_COMPARE_AMOUNT) {
            return;
        }

        playAlarmSignalSeconds(10);
        noTone(TONE_PIN);
        if (tDelay < 600) { // up to 5 minutes
            tDelay += tDelay / 16;
        }

    }
}

void playDoubleClick() {
    PWMtone(OPEN_WINDOW_ALARM_FREQUENCY_HIGH, 2);
    delayMilliseconds(100); // delay between clicks
    PWMtone(OPEN_WINDOW_ALARM_FREQUENCY_HIGH, 2);
}

/*
 * Flash LED only for a short period to save power.
 * If open window detected, increase pulse length to give a visual feedback
 */
void delayAndSignalOpenWindowDetectionAndLowVCC() {
    if (sOpenWindowDetected) {
        sOpenWindowDetectedOld = true;
        PWMtone(OPEN_WINDOW_ALARM_FREQUENCY_HIGH, 2); // 2 ms can be heard as click
        delayMicroseconds(20000); // to let the led light longer

    } else if (sOpenWindowDetectedOld) {
// closing window just detected -> signal it with 2 clicks
        sOpenWindowDetectedOld = false; // do it once
        playDoubleClick();

    } else if (sVCCVoltageTooLow) {
        PWMtone(OPEN_WINDOW_ALARM_FREQUENCY_VCC_TOO_LOW, 2); // Use a different frequency to distinguish the this alert from others
    } else {
        delayMicroseconds(LED_PULSE_LENGTH - 150);  // - 150 for the duration from digitalWrite(LED_PIN, 1) until here
    }
}

void sleepDelay(uint16_t aSecondsToSleep) {
    ADCSRA = 0; // disable ADC -> saves 150 - 200 uA
    for (uint16_t i = 0; i < (aSecondsToSleep / 8); ++i) {
        sleep_cpu()
        ;
    }
}

void delayMilliseconds(unsigned int aMillis) {
    for (unsigned int i = 0; i < aMillis; ++i) {
        delayMicroseconds(1000);
    }
}

#define ADC_PRESCALE8    3 // 104 microseconds per ADC conversion at 1 MHz
uint16_t readADCChannelWithReferenceOversample(uint8_t aChannelNumber, uint8_t aReference, uint8_t aOversampleExponent) {
    uint16_t tSumValue = 0;
    ADMUX = aChannelNumber | (aReference << SHIFT_VALUE_FOR_REFERENCE);

// ADCSRB = 0; // free running mode if ADATE is 1 - is default
// ADSC-StartConversion ADATE-AutoTriggerEnable ADIF-Reset Interrupt Flag
    ADCSRA = (_BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIF) | ADC_PRESCALE8);

    for (uint8_t i = 0; i < _BV(aOversampleExponent); i++) {
        /*
         * wait for free running conversion to finish.
         * Do not wait for ADSC here, since ADSC is only low for 1 ADC Clock cycle on free running conversion.
         */
        loop_until_bit_is_set(ADCSRA, ADIF);

        ADCSRA |= _BV(ADIF); // clear bit to recognize conversion has finished
// Add value
        tSumValue += ADCL | (ADCH << 8);
//        tSumValue += (ADCH << 8) | ADCL; // this does NOT work!
    }
    ADCSRA &= ~_BV(ADATE); // Disable auto-triggering (free running mode)
    return (tSumValue >> aOversampleExponent);
}

uint16_t getVCCVoltageMillivolt(void) {
// use AVCC with external capacitor at AREF pin as reference
    uint8_t tOldADMUX = ADMUX;
    /*
     * Must wait >= 200 us if reference has to be switched to VSS
     * Must wait >= 70 us if channel has to be switched to ADC_1_1_VOLT_CHANNEL_MUX
     */
    if ((ADMUX & (INTERNAL << SHIFT_VALUE_FOR_REFERENCE)) || ((ADMUX & 0x0F) != ADC_1_1_VOLT_CHANNEL_MUX)) {
// switch AREF
        ADMUX = ADC_1_1_VOLT_CHANNEL_MUX | (DEFAULT << SHIFT_VALUE_FOR_REFERENCE);
// and wait for settling
        delayMicroseconds(400); // experimental value is > 200 us
    }
    uint16_t tVCC = readADCChannelWithReferenceOversample(ADC_1_1_VOLT_CHANNEL_MUX, DEFAULT, 2);
    ADMUX = tOldADMUX;
    /*
     * Do not wait for reference to settle here, since it may not be necessary
     */
    return ((1024L * 1100) / tVCC);
}

/*
 * called every hour
 * needs 4.5 ms
 */
void checkVCCPeriodically() {
    sVCCMonitoringDelayCounter--;
    if (sVCCMonitoringDelayCounter == 0) {
        sVCCVoltageMillivolt = getVCCVoltageMillivolt();
#ifdef DEBUG
        Serial.print(F("VCC="));
        Serial.print(sVCCVoltageMillivolt);
        Serial.print(F("mV - "));
        if (sLIPOSupplyDetected) {
            Serial.print(F("LIPO"));
        } else {
            Serial.print(F("standard or button cell"));
        }
        Serial.println(" detected");
#endif
        if ((sLIPOSupplyDetected && sVCCVoltageMillivolt < VCC_VOLTAGE_LOWER_LIMIT_MILLIVOLT_LIPO)
                || (!sLIPOSupplyDetected && sVCCVoltageMillivolt < VCC_VOLTAGE_LOWER_LIMIT_MILLIVOLT_STANDARD)) {
            sVCCVoltageTooLow = true;
            sVCCMonitoringDelayCounter = 4; // VCC too low -> check every 2 minutes
        } else {
            sVCCVoltageTooLow = false;
            sVCCMonitoringDelayCounter = (VCC_MONITORING_DELAY_MIN * 60) / TEMPERATURE_SAMPLE_SECONDS; // VCC OK -> check every hour
        }
    }
}

/*
 * Watchdog wakes CPU periodically and all we have to do is call sleep_cpu();
 * aWatchdogPrescaler (see wdt.h) can be one of
 * WDTO_15MS, 30, 60, 120, 250, WDTO_500MS
 * WDTO_1S to WDTO_8S
 */
void initPeriodicSleepWithWatchdog(uint8_t tSleepMode, uint8_t aWatchdogPrescaler) {
    sleep_enable()
    ;
    set_sleep_mode(tSleepMode);
    MCUSR = ~_BV(WDRF); // Clear WDRF in MCUSR

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define WDTCSR  WDTCR
#endif
// Watchdog interrupt enable + reset interrupt flag -> needs ISR(WDT_vect)
    uint8_t tWDTCSR = _BV(WDIE) | _BV(WDIF) | (aWatchdogPrescaler & 0x08 ? _WD_PS3_MASK : 0x00) | (aWatchdogPrescaler & 0x07); // handles that the WDP3 bit is in bit 5 of the WDTCSR register,
    WDTCSR = _BV(WDCE) | _BV(WDE); // clear lock bit for 4 cycles by writing 1 to WDCE AND WDE
    WDTCSR = tWDTCSR; // set final Value
}

/*
 * This interrupt wakes up the cpu from sleep
 */
ISR(WDT_vect) {
    ;
}
