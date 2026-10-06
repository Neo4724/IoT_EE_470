/*
 * Title: LiPo Battery Voltage Logger
 *
 * Purpose:
 * Measure a 3.7 V LiPo battery with a maximum voltage of 4.2 V.
 * A voltage divider reduces the battery voltage so the ESP8266
 * ADC input never exceeds 3.3 V.
 *
 * Voltage Divider:
 * Battery + ---- R1 (985 ohm) ---- A0 ---- R2 (3310 ohm) ---- GND
 *
 * Measurements are taken every 60 seconds and printed
 * through Serial at 115200 baud.
 *
 * Author: Antonio Kassis
 * Version: 1.0
 */
#include<Arduino.h>
#define BATTERY_PIN A0

// Voltage divider resistor values
const float R1 = 1980.0;
const float R2 = 3310.0;

// ADC values
const float ADC_MAX_VOLTAGE = 3.3;
const float ADC_MAX_VALUE = 1023.0;

// ADC correction factor
// Corrects about 2.69 V down to about 2.52 V
const float ADC_SCALE = 0.9395;

// Number of ADC samples averaged together
const int NUM_SAMPLES = 20;

// One measurement every 60 seconds
const unsigned long SAMPLE_INTERVAL = 60000;

// Timing variables
unsigned long previousSampleTime = 0;
unsigned long sampleNumber = 0;


void setup() {

    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("LiPo Battery Voltage Logger");
    Serial.println("Sample,Time_min,Raw_ADC,ADC_Voltage,Battery_Voltage");

    // Take the first reading immediately
    previousSampleTime = millis() - SAMPLE_INTERVAL;
}


void loop() {

    unsigned long currentTime = millis();

    // Check if 60 seconds have passed
    if (currentTime - previousSampleTime >= SAMPLE_INTERVAL) {

        previousSampleTime = currentTime;
        sampleNumber++;

        float adcTotal = 0;

        // Take 20 ADC readings and average them
        for (int i = 0; i < NUM_SAMPLES; i++) {

            adcTotal += analogRead(BATTERY_PIN);

            delay(10);
        }

        // Average raw ADC reading
        float adcAverage = adcTotal / NUM_SAMPLES;

        // Convert ADC reading to voltage at A0
        // and apply correction factor
        float adcVoltage =
            adcAverage *
            (ADC_MAX_VOLTAGE / ADC_MAX_VALUE) *
            ADC_SCALE;

        // Convert A0 voltage back to actual battery voltage
        float batteryVoltage =
            adcVoltage *
            ((R1 + R2) / R2);

        // Calculate elapsed time in minutes
        float elapsedMinutes =
            currentTime / 60000.0;

        // Print data in CSV format
        Serial.print(sampleNumber);
        Serial.print(",");

        Serial.print(elapsedMinutes, 2);
        Serial.print(",");

        Serial.print(adcAverage, 1);
        Serial.print(",");

        Serial.print(adcVoltage, 3);
        Serial.print(",");

        Serial.println(batteryVoltage, 3);
    }
}