// Changelog:
// 12/3: Added sending to rs485
// 4/2: Add load cell sensor reading

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <Adafruit_ADS1X15.h>
#include <HardwareSerial.h>
#include <HX711.h>
#include <string>

#include "SPI.h"
using namespace std;

//Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

// ETHERNET CONNECTION
#define RO_PIN 16
#define DI_PIN 17
#define DE_RE_PIN 23

HardwareSerial rs485Serial(2);

// Number of Sensors
const int NUM_PT = 3;
const int NUM_LC = 1;
const int TOTAL_SENSORS = NUM_PT + NUM_LC;

// HX711 LOAD CELL PINS
const int LC1_DOUT_PIN = 2;
const int LC1_SCK_PIN = 15;

// LOAD CELLS
HX711 LC1;

// Queue to share sensor data between tasks
QueueHandle_t sensorQueue;

// Task to read sensor data
void readSensorsTask(void *pvParameters) {
    float sensorData[TOTAL_SENSORS];
    while (true) {
        // Read from pressure sensors
        for (int i = 0; i < NUM_PT; i++) {
            float raw = ads.readADC_SingleEnded(i); //Comment this if running simulation
            sensorData[i] = ads.computeVolts(raw);
            // sensorData[i] = 2.0; //Comment this if running real ADC
        }
        // Read from load cell sensors
        if (LC1.is_ready()) {
            sensorData[NUM_PT] = LC1.read();
        } else {
            sensorData[NUM_PT] = -1;  // Indicate load cell error
        }
        // Send data to the queue
        xQueueSend(sensorQueue, &sensorData, portMAX_DELAY); 
    }
}

// Task to combine and print sensor data
void combineAndPrintTask(void *pvParameters) {
    float receivedData[TOTAL_SENSORS];
    while (true) {
        if (xQueueReceive(sensorQueue, &receivedData, portMAX_DELAY) == pdPASS) {
            // Combine data into a string
            String storeStr = "Asensorvals ";
            for (int i = 0; i < NUM_PT; i++) {
                storeStr += "pt" + String(i + 1) + "=" + String(receivedData[i]) + ",";
            }
            for (int i = NUM_PT; i < TOTAL_SENSORS; i++) {
                storeStr += "lc" + String(i + 1) + "=" + String(receivedData[i]) + ",";
            }
            storeStr.remove(storeStr.length() - 2); // Remove trailing comma
            storeStr+="Z";
            // digitalWrite(DE_RE_PIN, HIGH); // do we need this?
            rs485Serial.println(storeStr);
            Serial.println(storeStr);
        }
    }
}

void setup() {
    Serial.begin(115200);

    Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

    // The ADC input range (or gain) can be changed via the following
    // functions, but be careful never to exceed VDD +0.3V max, or to
    // exceed the upper and lower limits if you adjust the input range!
    // Setting these values incorrectly may destroy your ADC!
    //                                                                ADS1015  ADS1115
    //                                                                -------  -------
    // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
    // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
    // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
    // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
    // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

    // Initialize ADS1015;
    if (!ads.begin()) {
        Serial.println("Failed to initialize ADS.");
        while (1);
    }

    // Load Cell Measurement
    LC1.begin(LC1_DOUT_PIN, LC1_SCK_PIN);
    //LC1.set_scale(LC1_calibration);
    //LC1.tare(); // reset load cell to 0

    //Ethernet Connection
    pinMode(DE_RE_PIN, OUTPUT);
    digitalWrite(DE_RE_PIN, HIGH);

    // put your setup code here, to run once:
    rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);

    // Create a queue to hold sensor data
    sensorQueue = xQueueCreate(1, sizeof(float) * TOTAL_SENSORS);

    // 2048 is the number of bytes allocated to the heap, may need to increase if neccessary
    xTaskCreatePinnedToCore(readSensorsTask, "ReadSensors", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(combineAndPrintTask, "CombineAndPrint", 2048, NULL, 1, NULL, 1);
}

void loop() {
    // Empty. Tasks handle everything.
}
