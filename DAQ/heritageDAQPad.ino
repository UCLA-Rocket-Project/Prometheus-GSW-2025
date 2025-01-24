#include <HardwareSerial.h>
#include <RP_ADS1256.h>
#include <HX711.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

// GLOBAL CONSTANTS
const int NUM_PT = 6;
const int NUM_LC = 2;
const int NUM_TC = 2;

/*
#include <Adafruit_MAX31856.h>
#include <SPI.h>
#include <Wire.h>*/


// ETHERNET CONNECTION
#define RO_PIN 16
#define DI_PIN 17
#define DE_RE_PIN 23

HardwareSerial rs485Serial(2);


// PRESSURE MEASUREMENT
#define ADS1256_SCK 27
#define ADS1256_MISO 36
#define ADS1256_MOSI 14
#define ADS1256_CS 13
#define ADS1256_DRDY 15

ADS1256 PTADC(ADS1256_DRDY, 0, 0, ADS1256_CS, 2.500); //DRDY, RESET, SYNC(PDWN), CS, VREF(float). 


// FORCE MEASUREMENT
#define LC1_DOUT 32  // Tension Load Cell
#define LC1_SCK 33
#define LC2_DOUT 25  // Tension Load Cell
#define LC2_SCK 26

HX711 LC1;
HX711 LC2;

// LC calibration factors
const float LC1_calibration = 995;    // Pulling is positive, pushing is negative
const float LC2_calibration = -965;   // Pulling is positive, pushing is negative
// OLD VALUE: const float LC2_calibration = 2105;   // Pulling is positive, pushing is negative


// SD CARD READER
#define SDCARD_SCK_PIN 19
#define SDCARD_MISO_PIN 18
#define SDCARD_MOSI_PIN 5
#define SDCARD_SS_PIN 4

String sd_fileName = "/data.csv";

// Functions prototypes for file read/write
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);

/*
// IO Expander (TO CHANGE)
#define IOEXPANDER_CSPIN 3
#define dump 4
#define vent 5
#define qd 6
#define mpv 7

Adafruit_MCP23X17 mcp;
*/

/*
// Temperature Measurement (CHANGE CS NUMBER)
// To use software SPI, we need to pass in CS, DI, DO, CLK
Adafruit_MAX31856 tc1 = Adafruit_MAX31856(??, ??, ??, ??); // Type T TC
Adafruit_MAX31856 tc2 = Adafruit_MAX31856(??, ??, ??, ??); // Type K TC
*/

// Data Sending Interval Settings
unsigned long long delay_time = 200;
unsigned long long last_time = 0;

void setup() {

  // ETHERNET CONNECTION
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, HIGH);

  int baud = 74880;

  rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);
  Serial.begin(9600);


  // PRESSURE MEASUREMENT
  // Initialize SPI object for ADS SPI communication
  SPIClass* ADS_SPI = new SPIClass(VSPI);
  ADS_SPI->begin(ADS1256_SCK, ADS1256_MISO, ADS1256_MOSI, ADS1256_CS);

  // Initialize ADS1256
  PTADC.InitializeADC(ADS_SPI);


  // FORCE MEASUREMENT
  LC1.begin(LC1_DOUT, LC1_SCK);
  LC1.set_scale(LC1_calibration);
  LC1.tare(); // Reset load cell to 0

  LC2.begin(LC2_DOUT, LC2_SCK);
  LC2.set_scale(LC2_calibration);
  LC2.tare(); // Reset load cell to 0


  // SD CARD READER
  SPIClass* SD_SPI = new SPIClass(HSPI);
  SD_SPI->begin(SDCARD_SCK_PIN, SDCARD_MISO_PIN, SDCARD_MOSI_PIN);

  if(!SD.begin(SDCARD_SS_PIN, *SD_SPI)){
    Serial.println("Card Mount Failed");
    return;
  }

  // Construct header row for csv file
  std::string dataHeader;
  for(int i=0; i<NUM_PT; i++) dataHeader += "P" + std::to_string(i+1) + ",";
  for(int i=0; i<NUM_LC; i++) dataHeader += "L" + std::to_string(i+1) + ",";
  for(int i=0; i<NUM_TC; i++) dataHeader += "T" + std::to_string(i+1) + (i==NUM_TC-1 ? "\n": ",");
  writeFile(SD, sd_fileName.c_str(), dataHeader.c_str());


  /*
  // IO EXPANDER (TO CHANGE)
  Adafruit_MCP23X17 mcp;
  if (!mcp.begin_SPI(IOEXPANDER_CSPIN)) {
    Serial.println("IO Expander Initialization Error");
    while (1);
  }
  */

  /*
  pinMode(fill, INPUT);
  pinMode(dump, INPUT);
  pinMode(vent, INPUT);
  pinMode(qd, INPUT);
  pinMode(mpv, INPUT);
  */

  /*
  // TEMPERATURE MEASUREMENT
  // Initialize TCs
  if (!tc1.begin()) {
    Serial.println("Could not initialize tc1.");
    while (1) delay(10);
  }
  if (!tc2.begin()) {
    Serial.println("Could not initialize tc2.");
    while (1) delay(10);
  }
  // Set type
  tc1.setThermocoupleType(MAX31856_TCTYPE_T);
  tc2.setThermocoupleType(MAX31856_TCTYPE_K);
  // Set mode
  tc1.setConversionMode(MAX31856_CONTINUOUS);
  tc2.setConversionMode(MAX31856_CONTINUOUS);
  */

  last_time = millis();
}

void loop() 
{

  if((millis() - last_time) > delay_time)
  {
      // Create variables for all sensors
    float ptVals[6], lcVals[2], tcVals[2]; // TC1: Type T TC, TC2: Type K TC
    String printStr = String(millis()) + " ";
    String storeStr = String(millis()) + ",";

    // Read Pressure Transducer values
    for (int i = 0; i < 8; i++) //Why is this 8??
    {
      long rawConversion = PTADC.cycleSingle();
      float voltageValue = PTADC.convertToVoltage(rawConversion);

      if(i >= NUM_PT) continue;
      ptVals[i] = voltageValue;
    }

    // Read Load Cell values
    lcVals[0] = LC1.get_units();
    lcVals[1] = LC2.get_units();
    
    /*
    // Read Thermocouple values
    tc1Val = tc1.readThermocoupleTemperature();
    tc2Val = tc2.readThermocoupleTemperature();
    */

    // IO Expander
    
  
    // Calibration for PTs (likely have to calibrate everytime you flow)
    ptVals[0] = ptVals[0] * 107 - 63.5;
    ptVals[1] = ptVals[1] * 420 - 251;
    ptVals[2] = ptVals[2] * 421 - 245;
    ptVals[3] = ptVals[3] * 420 - 259;
    ptVals[4] = ptVals[4] * 418 - 237;
    ptVals[5] = ptVals[5] * 421 - 244; //copied from ptVal[4]
  
    // Sending over Ethernet cable (convert all data to Strings)
    // digitalWrite(DE_RE_PIN, HIGH);
    // delay(1);

    
    /*
    String pt1vals, pt2vals, pt3vals, pt4vals, pt5vals, pt6vals, lc1vals, lc2vals, lc3vals, tc1vals, tc2vals, connectionStatuss;
    String fillAcc, dumpAcc, ventAcc, ignite1Acc, qdAcc, mpvAcc, ignite2Acc, ignite3Acc;
    */

    
    for(int i = 0; i < NUM_PT; i++)
    {
      printStr += "PT" + String(i+1) + ": " + String(int(ptVals[i])) + ", ";
      storeStr += String(ptVals[i]) + ",";
    }
    for(int i = 0; i < NUM_LC; i++)
    {
      printStr += "LC" + String(i+1) + ": " + String(lcVals[i]) + (i==0 ? ", " : "");
      storeStr += String(lcVals[i]) + ",";
    }

    // Printing to Serial
    Serial.println(printStr.c_str());

    // Sending via Ethernet Cable
    digitalWrite(DE_RE_PIN, HIGH);
    delay(50);
    printStr = 'A' + printStr + 'Z';
    rs485Serial.println(printStr.c_str());

    // Storing to microSD Card
    storeStr += "\n";
    appendFile(SD, sd_fileName.c_str(), storeStr.c_str());
    
    
    /*
    lc1vals = String(lc1Val);
    lc2vals = String(lc2Val);
    lc3vals = String(lc3Val);
    tc1vals = String(tc1Val);
    tc2vals = String(tc2Val);
  
    fillAcc = String(digitalRead(fill));
    dumpAcc = String(digitalRead(dump));
    ventAcc = String(digitalRead(vent));
    qdAcc = String(digitalRead(qd));
    mpvAcc = String(digitalRead(mpv));
  
    Serial.print("P1:");
    Serial.print(pt1vals);
    Serial.print(" P2:");
    Serial.print(pt2vals);
    Serial.print(" P3:");
    Serial.print(pt3vals);
    Serial.print(" P4:");
    Serial.print(pt4vals);
    Serial.print(" P5:");
    Serial.print(pt5vals);
    Serial.print(" LC1:");
    Serial.print(lc1vals);
    Serial.print(" AC1:");
    Serial.print(fillAcc);
    Serial.print(" AC2:");
    Serial.print(dumpAcc);
    Serial.print(" AC3:");
    Serial.print(ventAcc);
    Serial.print(" AC4:");
    Serial.print(qdAcc);
    Serial.print(" AC5:");
    Serial.println(mpvAcc);
    
    rs485Serial.print("P1:");
    rs485Serial.print(pt1vals);
    rs485Serial.print(" P2:");
    rs485Serial.print(pt2vals);
    rs485Serial.print(" P3:");
    rs485Serial.print(pt3vals);
    rs485Serial.print(" P4:");
    rs485Serial.print(pt4vals);
    rs485Serial.print(" P5:");
    rs485Serial.print(pt5vals);
    rs485Serial.print(" LC1:");
    rs485Serial.print(lc1vals);
    rs485Serial.print(" AC1:");
    rs485Serial.print(fillAcc);
    rs485Serial.print(" AC2:");
    rs485Serial.print(dumpAcc);
    rs485Serial.print(" AC3:");
    rs485Serial.print(ventAcc);
    rs485Serial.print(" AC4:");
    rs485Serial.print(qdAcc);
    rs485Serial.print(" AC5:");
    rs485Serial.println(mpvAcc); */


    last_time = millis();
  }
}

// MICROSD CARD READER FUNCTION DEFINITIONS

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








////////// LOADCELL CALIBRATION CODE //////////

/*
#include "HX711.h"

#define hx1_data_pin  45
#define hx_sck_pin  37

HX711 lc1;

float calibration_factor = 18980; // Torque load cell
//float calibration_factor = -2585;// Tension load cell

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  lc1.begin(hx1_data_pin, hx_sck_pin);
  lc1.set_scale();
  lc1.tare(); //Reset the scale to 0

  long zero_factor = lc1.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
}

void loop() {

  lc1.set_scale(calibration_factor); //Adjust to this calibration factor

  Serial.print("Reading: ");
  Serial.print(lc1.get_units());
  Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }
}*/
