// ARES EGSE 2024-2025 Load Cell System Code
// ESP32-S3 Documentation: https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
// ADS1256 Documentation: https://www.ti.com/lit/gpn/ads1256
// ADS1256 Library Information: https://github.com/CuriousScientist0/ADS1256/tree/main

#include <Arduino.h>
#include <ADS1256.h>

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

// Define custom SPI pins for ADS1256 Module
#define ADS_MISO 13
#define ADS_SCK 12
#define ADS_MOSI 11
#define ADS_CS  10

#define DRDY 4

SPIClass custom_spi; // Create custom SPI instance
//SPIClass custom_spi(FSPI); // Create custom SPI instance
ADS1256 ADC(&custom_spi, DRDY, ADS_CS, 2.5); // Instantiate ADS1256

// Calibration coefficients y=Ax+B
float calibrationA = -195.25664;
float calibrationB = -91.19858;

// convertToWeight function
// Returns measured force (lbf)
float convertToWeight(float voltage) {
  return (calibrationA * voltage) + calibrationB;
}

// SPI pins for SD card on HSPI
#define SD_SCK   12  // Replace with your HSCK pin
#define SD_MISO  13  // Replace with your HMISO pin
#define SD_MOSI  11  // Replace with your HMOSI pin
#define SD_CS    6  // Replace with your CS_XTSD pin

String filename;

String makeFile() {
  int index = 0;
  String path;

  // Look for the first filename that does NOT exist
  while (true) {
    path = "/data" + String(index) + ".txt";
    if (!SD.exists(path)) {
      break;
    }
    index++;
  }
  return path;
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void setup()
{
    // Initialize Serial
    Serial.begin(115200);
    delay(100);

    // Configure SPI pins
    pinMode(ADS_MISO, INPUT_PULLUP);
    pinMode(ADS_SCK, OUTPUT);
    pinMode(ADS_MOSI, OUTPUT);

    Serial.print("Initializing SD card...");

    // Initialize the SPI bus with custom pins
    custom_spi.begin(SD_SCK, SD_MISO, SD_MOSI, -1);

    // setup and write to the SD Card
    if (!SD.begin(SD_CS, &custom_spi)) {
        Serial.println("Failed to initialize SD card");
        while(1);
    }

    Serial.println("SD initialization done.");
    filename = makeFile();
    writeFile(SD, filename.c_str(), "TESTING\n");

    // setup and write to the ADS1256
    custom_spi.begin(ADS_SCK, ADS_MISO, ADS_MOSI, -1);
    if (!custom_spi.begin(ADS_CS, &custom_spi)) {
        Serial.println("Failed to initialize ADS SPI");
        while(1);
    }

    // ADC Setup
    ADC.InitializeADC(); 

    // Set Gain (PGA)
    ADC.setPGA(PGA_64);  // Maximum gain for small signals

    //Set Mux
    ADC.setMUX(DIFF_0_1);

    // Set Sampling Rate
    ADC.setDRATE(DRATE_1000SPS);  

    delay(100);
}

void loop() {
  float voltages[2]; // Store two differential voltages

  // ----- Read AIN0-AIN1 -----
  ADC.setMUX(DIFF_0_1);         // Set MUX to AIN0-AIN1
  voltages[0] = ADC.convertToVoltage(ADC.readSingle()); // Read value and convert to voltage

  // ----- Read AIN2-AIN3 -----
  ADC.setMUX(DIFF_2_3);         // Set MUX to AIN2-AIN3
  voltages[1] = ADC.convertToVoltage(ADC.readSingle());

  // ----- Print Results -----
  Serial.print("AIN0-1: ");
  Serial.print(voltages[0], 6);
  Serial.print(" V\t");

  Serial.print("AIN2-3: ");
  Serial.print(voltages[1], 6);
  Serial.println(" V");

  String message = "helloworld\n";

  //Serial.print(message);
  Serial.print(message.c_str());
  
  appendFile(SD, filename.c_str(), message.c_str());
  readFile(SD, filename.c_str());
 
  delay(1000);
}
