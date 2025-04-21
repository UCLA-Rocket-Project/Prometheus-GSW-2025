// ARES EGSE 2024-2025 Load Cell System Code
// ESP32-S3 Documentation: https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
// ADS1256 Documentation: https://www.ti.com/lit/gpn/ads1256
// ADS1256 Library Information: https://github.com/CuriousScientist0/ADS1256/tree/main

#include <Arduino.h>
#include <ADS1256.h>

// Define custom SPI pins for ADS1256 Module
#define SPI_MISO 37
#define SPI_SCLK 36
#define SPI_MOSI 35
#define SPI_CS  34

#define DRDY 38

// Define LED indicator pin
#define LED 7

SPIClass spi_ads(FSPI); // Create custom SPI instance
ADS1256 ADC(&spi_ads, DRDY, SPI_CS, 2.5); // Instantiate ADS1256

// Calibration coefficients y=Ax+B
float calibrationA = -195.25664;
float calibrationB = -91.19858;

// convertToWeight function
// Returns measured force (lbf)
float convertToWeight(float voltage) {
  return (calibrationA * voltage) + calibrationB;
}

void setup()
{
    // Initialize Serial
    Serial.begin(115200);
    delay(100);

    // Configure SPI pins
    pinMode(SPI_MISO, INPUT_PULLUP);
    pinMode(SPI_SCLK, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);

    // Configure LED pin and write HIGH
    pinMode(LED, OUTPUT); digitalWrite(LED, HIGH);

    // Initialize the SPI bus with custom pins
    spi_ads.begin(SPI_SCLK, SPI_MISO, SPI_MOSI, SPI_CS);

    // ADC Setup
    ADC.InitializeADC(); 

    // Set Gain (PGA)
    ADC.setPGA(PGA_64);  // Maximum gain for small signals

    // Set Differential Input Channels (AIN0 - AIN1)
    ADC.setMUX(DIFF_0_1);  

    // Set Sampling Rate
    ADC.setDRATE(DRATE_1000SPS);  

    delay(100);
}

void loop() {
  static int sampleCount = 0;
  static float voltageSamples[10];
  static float weightSamples[10];

  // Read differential voltage
  long raw = ADC.readSingleContinuous();
  float voltage = ADC.convertToVoltage(raw) * 1000; // mV

  // Print raw voltage reading
  Serial.print("Voltage (mV): ");
  Serial.println(voltage, 5);

  // If user inputs a known weight
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "calc") {
      // Perform linear regression
      if (sampleCount < 2) {
        Serial.println("Need at least 2 points.");
        return;
      }

      float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
      for (int i = 0; i < sampleCount; i++) {
        sumX += voltageSamples[i];
        sumY += weightSamples[i];
        sumXY += voltageSamples[i] * weightSamples[i];
        sumX2 += voltageSamples[i] * voltageSamples[i];
      }

      float A = (sampleCount * sumXY - sumX * sumY) / (sampleCount * sumX2 - sumX * sumX);
      float B = (sumY - A * sumX) / sampleCount;

      Serial.print("Calibration A: ");
      Serial.println(A, 5);
      Serial.print("Calibration B: ");
      Serial.println(B, 5);

      calibrationA = A;
      calibrationB = B;
    } else {
      // Convert user input to float (expected to be weight in lbf)
      float knownWeight = input.toFloat();
      if (sampleCount < 10) {
        voltageSamples[sampleCount] = voltage;
        weightSamples[sampleCount] = knownWeight;
        sampleCount++;

        Serial.print("Recorded point ");
        Serial.print(sampleCount);
        Serial.print(": ");
        Serial.print(voltage, 5);
        Serial.print(" mV => ");
        Serial.print(knownWeight, 2);
        Serial.println(" lbf");
      } else {
        Serial.println("Max samples reached. Type 'calc' to calculate calibration.");
      }
    }
  }

  delay(1000); // Slow down output
}
