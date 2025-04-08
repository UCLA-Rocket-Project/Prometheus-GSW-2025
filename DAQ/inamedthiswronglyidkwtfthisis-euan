#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
// #include <Adafruit_ADS1X15.h>
#include <string>
using namespace std;

// Adafruit_ADS1015 ads;

// Number of sensors
const int NUM_PT = 6;

// Queue to share sensor data between tasks
QueueHandle_t sensorQueue;

// Task to read sensor data
void readSensorsTask(void *pvParameters) {
    float sensorData[NUM_PT];
    while (true) {
        // Read from each sensor
        for (int i = 0; i < NUM_PT; i++) {
            // float raw = ads.readADC_SingleEnded(i); //Comment this if running simulation
            // sensorData[i] = ads.computeVolts(raw);
            sensorData[i] = 2.0; //Comment this if running real ADC
        }
        // Send data to the queue
        xQueueSend(sensorQueue, &sensorData, portMAX_DELAY); 
    }
}

// Task to combine and print sensor data
void combineAndPrintTask(void *pvParameters) {
    float receivedData[NUM_PT];
    while (true) {
        if (xQueueReceive(sensorQueue, &receivedData, portMAX_DELAY) == pdPASS) {
            // Combine data into a string
            String storeStr = "Asensorvals ";
            for (int i = 0; i < NUM_PT; i++) {
                storeStr += "pt" + String(i + 1) + "=" + String(receivedData[i]) + ",";
            }
            storeStr.remove(storeStr.length() - 2); // Remove trailing comma
            storeStr+=",lc1=100,lc2=300Z";
            Serial.println(storeStr);
        }
    }
}

void setup() {
    Serial.begin(9600);
    // Initialize ADS1015
    // if (!ads.begin()) {
    //     Serial.println("Failed to initialize ADS.");
    //     while (1);
    // }

    // Create a queue to hold sensor data
    sensorQueue = xQueueCreate(1, sizeof(float[NUM_PT]));

    // 2048 is the number of bytes allocated to the heap, may need to increase if neccessary
    xTaskCreatePinnedToCore(readSensorsTask, "ReadSensors", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(combineAndPrintTask, "CombineAndPrint", 2048, NULL, 1, NULL, 1);
}

void loop() {
    // Empty. Tasks handle everything.
}
