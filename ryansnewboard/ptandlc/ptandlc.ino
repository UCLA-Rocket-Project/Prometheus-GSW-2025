#include <Arduino.h>
#include <ADS1256.h>
#include "ADS8688.h"
#include <SPI.h>

// Load Cell (ADS1256) SPI Pins
#define ADS1256_MISO 35
#define ADS1256_SCLK 48
#define ADS1256_MOSI 34
#define ADS1256_CS   7
#define ADS1256_DRDY 4

// PT (ADS8688) SPI Pin
#define ADS8688_CS 36

// LED indicator pin
#define LED 38

// SPI bus shared between both ADCs
SPIClass sharedSPI(FSPI);

// ADS1256 instance
ADS1256 loadCellADC(&sharedSPI, ADS1256_DRDY, ADS1256_CS, 2.5);

// ADS8688 instance
ADS8688 pressureADC;

// Calibration coefficients for load cell
float calibrationA = -195.25664;
float calibrationB = -91.19858;

// Convert voltage to weight
float convertToWeight(float voltage) {
  return (calibrationA * voltage) + calibrationB;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // Start custom SPI bus
  sharedSPI.begin(ADS1256_SCLK, ADS1256_MISO, ADS1256_MOSI, -1);

  // Initialize ADS1256 (Load Cell)
  loadCellADC.InitializeADC();
  loadCellADC.setPGA(PGA_64);
  loadCellADC.setMUX(DIFF_0_1);
  loadCellADC.setDRATE(DRATE_1000SPS);

  // Initialize ADS8688 (PTs)
  pressureADC.begin(ADS1256_MISO, ADS1256_SCLK, ADS1256_MOSI, ADS8688_CS, 4.1, 0x05);
  pressureADC.setInputRange(ADS8688_CS, 0x05);

  Serial.println("Setup complete");
}

void loop() {
  // --- Load Cell Measurements (AIN0-AIN1 & AIN2-AIN3) ---
  float loadVoltages[2];
  loadCellADC.setMUX(DIFF_0_1);
  loadVoltages[0] = loadCellADC.convertToVoltage(loadCellADC.readSingle());

  loadCellADC.setMUX(DIFF_2_3);
  loadVoltages[1] = loadCellADC.convertToVoltage(loadCellADC.readSingle());

  Serial.print("Load AIN0-1: "); Serial.print(loadVoltages[0], 6); Serial.print(" V\t");
  Serial.print("AIN2-3: "); Serial.print(loadVoltages[1], 6); Serial.println(" V");

  // --- PT Measurements (8 channels) ---
  float ptVoltages[8];
  float ptCalibrated[8];
  pressureADC.readAllChannels(ADS8688_CS, true, ptVoltages);

  for (int i = 0; i < 8; i++) {
    ptCalibrated[i] = 0.5f * ptVoltages[i];
  }

  Serial.print("PTs: ");
  for (int i = 0; i < 7; i++) {
    Serial.print(ptCalibrated[i], 2); Serial.print(",");
  }
  Serial.println(ptCalibrated[7], 2);

  delay(100);
}
