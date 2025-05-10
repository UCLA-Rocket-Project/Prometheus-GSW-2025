
// UPDATED: 5.9.25 - WORKING

// ARES EGSE 2024-2025 Load Cell System Code
// ESP32-S3 Documentation: https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
// ADS1256 Documentation: https://www.ti.com/lit/gpn/ads1256
// ADS1256 Library Information: https://github.com/CuriousScientist0/ADS1256/tree/main

// #include <Arduino.h>
#include <ADS1256.h>

// #include <Adafruit_ADS1X15.h>
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

SPIClass custom_spi_bus;
//SPIClass spi_ads(FSPI); // Create custom SPI instance
ADS1256 ADC(&custom_spi_bus, DRDY, ADS_CS, 2.5); // Instantiate ADS1256

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
    path = "/launch" + String(index) + ".txt";
    if (!SD.exists(path)) {
      break;
    }
    index++;
  }
  Serial.printf("Created filename: %s\n", path);
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

    custom_spi_bus.begin(ADS_SCK, ADS_MISO, ADS_MOSI, -1);

    // Initialize the SPI bus with custom pins
    // spi_ads.begin(SPI_SCLK, SPI_MISO, SPI_MOSI, SPI_CS);

    // ADC Setup
    ADC.InitializeADC(); 

    // Set Gain (PGA)
    ADC.setPGA(PGA_1);  // Maximum gain for small signals

    //Set Mux
    ADC.setMUX(SING_0);

    // Set Sampling Rate
    ADC.setDRATE(DRATE_1000SPS);  

    delay(100);

  Serial.print("Initializing SD card...");

  bool sd_init = false;

  while (!sd_init) {
    // SPI.begin(SD_HSCK, SD_HMISO, SD_HMOSI, SD_CS_XTSD);
    
    sd_init = SD.begin(SD_CS, custom_spi_bus);
//    if (!sd_init) {
//      Serial.println("initialization failed!");
//    }
//    if (!SD.begin()) {
//      Serial.println("initialization still failed!");
//    }
    if (!sd_init) {
    Serial.println("Waiting for SD card... Insert card now.");
    delay(1000); // wait and retry
    }
  }

  Serial.println("SD initialization done.");

  filename = makeFile();
  writeFile(SD, filename.c_str(), "Timestamp,PT1,PT2,PT3,V1,V2,V3\n");

  // readFile(SD, "/launch49.txt");

}

void loop() {
  long pt1 = 0;
  long pt2 = 0;
  long pt3 = 0; //pts
  float v1 = 0;
  float v2 = 0;
  float v3 = 0; //volts

  ADC.setMUX(SING_0);
  pt1 = ADC.readSingle(); //reads A0
  // Serial.println("pt1 is " + pt1);
  v1 = ADC.convertToVoltage(pt1);

  ADC.setMUX(SING_1);
  pt2 = ADC.readSingle(); //reads A1
  v2 = ADC.convertToVoltage(pt2);
  ADC.setMUX(SING_2);
  pt3 = ADC.readSingle(); //reads A2
  v3 = ADC.convertToVoltage(pt3);
  
//  Serial.print("PT1: "); Serial.print(pt1); Serial.print(" = "); Serial.print(v1); Serial.println("V");
//  Serial.print("PT2: "); Serial.print(pt2); Serial.print(" = "); Serial.print(v2); Serial.println("V");
//  Serial.print("PT3: "); Serial.print(pt3); Serial.print(" = "); Serial.print(v3); Serial.println("V");
  
  //Serial.print("Writing to test.txt...");
  //String message = "\n";
//  message += "PT1: " + String(pt1) + " = " + String(v1) + "V\n";
//  message += "PT2: " + String(pt2) + " = " + String(v2) + "V\n";
//  message += "PT3: " + String(pt3) + " = " + String(v3) + "V\n";
  String message = String(millis()) + "," + String(pt1) + "," + String(pt2) + "," + String(pt3) + "," + String(v1) + "," + String(v2) + "," + String(v3) + "\n";

  //Serial.print(message);
  Serial.print(message.c_str());
  
  appendFile(SD, filename.c_str(), message.c_str());
  //readFile(SD, filename.c_str());
 
  delay(15);

  // String message = "helloworld\n";

  //Serial.print(message);
  // Serial.print(message.c_str());
  
  // appendFile(SD, filename.c_str(), message.c_str());
  // readFile(SD, filename.c_str());
 
  // delay(1000);
}