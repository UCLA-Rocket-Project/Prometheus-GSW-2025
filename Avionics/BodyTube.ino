#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>

// SPI pins for SD card on HSPI
#define SD_HSCK   14  // Replace with your HSCK pin
#define SD_HMISO  12  // Replace with your HMISO pin
#define SD_HMOSI  13  // Replace with your HMOSI pin
#define SD_CS_XTSD    15  // Replace with your CS_XTSD pin

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

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

void setup(void)
{
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // Configure the custom SPI pins
  Serial.println("Getting single-ended readings from AIN0..3");
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
  // ads.setGain(GAIN_ONE);

  Wire.begin(32, 33); // default should be 21 and 22 for ESP32 (SDA, SCL)
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
    
  Serial.print("Initializing SD card...");

  bool sd_init = false;
  while (!sd_init) {
    SPI.begin(SD_HSCK, SD_HMISO, SD_HMOSI, SD_CS_XTSD);
    sd_init = SD.begin(SD_CS_XTSD);
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
  
  writeFile(SD, filename.c_str(), "PT1,PT2,PT3,V1,V2,V3\n");

}

void loop(void)
{
  int16_t pt1, pt2, pt3; //pts
  float v1, v2, v3; //volts

  pt1 = ads.readADC_SingleEnded(0); //reads A0
  pt2 = ads.readADC_SingleEnded(1); //reads A1
  pt3 = ads.readADC_SingleEnded(2); //reads A2

  v1 = ads.computeVolts(pt1);
  v2 = ads.computeVolts(pt2);
  v3 = ads.computeVolts(pt3);

//  Serial.print("PT1: "); Serial.print(pt1); Serial.print(" = "); Serial.print(v1); Serial.println("V");
//  Serial.print("PT2: "); Serial.print(pt2); Serial.print(" = "); Serial.print(v2); Serial.println("V");
//  Serial.print("PT3: "); Serial.print(pt3); Serial.print(" = "); Serial.print(v3); Serial.println("V");
  
  //Serial.print("Writing to test.txt...");
  //String message = "\n";
//  message += "PT1: " + String(pt1) + " = " + String(v1) + "V\n";
//  message += "PT2: " + String(pt2) + " = " + String(v2) + "V\n";
//  message += "PT3: " + String(pt3) + " = " + String(v3) + "V\n";
  String message = String(pt1) + "," + String(pt2) + "," + String(pt3) + "," + String(v1) + "," + String(v2) + "," + String(v3) + ",";

  //Serial.print(message);
  //Serial.print(message.c_str());
  
  appendFile(SD, filename.c_str(), message.c_str());
  //readFile(SD, filename.c_str());
 
  delay(1000);
}