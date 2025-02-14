//TODO: INCLUDE MICROSD AND LOAD CELL CODE FOR TRANSMISSION

#include <string>
#include <Adafruit_ADS1X15.h>
#include <HardwareSerial.h>
#include "SPI.h"
#include "FS.h"
#include "SD.h"
using namespace std;

//Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */
//Number of pressure transducers
const int NUM_PT = 3;
// Data Sending Interval Settings
unsigned long long delay_time = 200;
unsigned long long last_time = 0;


// ETHERNET CONNECTION
#define RO_PIN 16
#define DI_PIN 17
#define DE_RE_PIN 23

// microSD card connection
#define SD_SCK 18 // connect this to CLK pin
#define SD_MISO 19 // connect this to DO pin
#define SD_MOSI 23 // connect this to the DI pin
#define SD_CS 5 // connect this to the CS pin

const int FILE_NAME_MAX_LENGTH = 50;
char new_file_name[FILE_NAME_MAX_LENGTH + 1];

HardwareSerial rs485Serial(2);

void setup() {

  //Ethernet Connection
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, HIGH);

  // put your setup code here, to run once:
  rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);
  Serial.begin(9600);
  last_time = millis();
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
  // if (!ads.begin()) {
  //   Serial.println("Failed to initialize ADS.");
  //   while (1);
  // }

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    
  // Start SD card
  if(!SD.begin(SD_CS, SPI, 4000000)){ // 4MHz SPI speed
      Serial.println("Card Mount Failed");
      return;
  }

  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
  }

  Serial.println("SD Card initialized.");
  getNewFilename(new_file_name);
  
  Serial.print("The new file's name is: ");
  Serial.println(new_file_name);
  writeFile(SD, new_file_name, "pt1,pt2,pt3");
}

void loop() {
  if((millis() - last_time) > delay_time){
    std::string string_to_write;

    float ptVals[NUM_PT];

    // Read Pressure Transducer values
    for (int i = 0; i < 3; i++)
    {
      float raw = ads.readADC_SingleEnded(i);
      ptVals[i] = ads.computeVolts(raw);

      string_to_write += std::to_string(raw);
      string_to_write += i < NUM_PT - 1 ? "," : "";
    }
    //  // Calibration for PTs (likely have to calibrate everytime you flow)
    ptVals[0] = ptVals[0] * 381.0376825 - 124.3444374;
    ptVals[1] = ptVals[1] * 381.2890799 - 120.337546;
    ptVals[2] = ptVals[2] * 380.6974487 - 114.0943018;
    // ptVals[3] = ptVals[3] * 420 - 259;

    //fake values to pass to GUI
    float pt5 = 0;
    float pt6 = 0;  
    float lc1 = 0;  
    float lc2 = 0;  

    String storeStr = "Asensorvals pt1=" + String(ptVals[0]) + 
                  ",pt2=" + String(ptVals[1]) + 
                  ",pt3=" + String(ptVals[2]) + 
                  ",pt4=" + String(ptVals[3]) + 
                  ",pt5=" + String(pt5) + 
                  ",pt6=" + String(pt6) + 
                  ",lc1=" + String(lc1) + 
                  ",lc2=" + String(lc2) + 
                  "Z";
    Serial.println(storeStr);
    digitalWrite(DE_RE_PIN, HIGH);
    delay(50);
    rs485Serial.println(storeStr);

    // set up the string for writing
    string_to_write += '\n';
    Serial.print("Writing: ");
    Serial.println(string_to_write.c_str());
    appendFile(SD, new_file_name, string_to_write.c_str());
  }
}

bool getNewFilename(char* new_file_name) {
  int counter = 0;
  char candidate_file_name[FILE_NAME_MAX_LENGTH + 1];
  while (true) {
    sprintf(candidate_file_name, "/data%d.csv", counter);
    Serial.print("Checking file: ");
    Serial.println(candidate_file_name);
    if (!SD.exists(candidate_file_name)) {
      sprintf(new_file_name, candidate_file_name);
      return true;
    }
    ++counter;
  }

  return false;
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}



