//TODO: INCLUDE MICROSD AND LOAD CELL CODE FOR TRANSMISSION

#include <string>
#include <Adafruit_ADS1X15.h>
#include <HardwareSerial.h>
#include <HX711.h>

#include "SPI.h"
using namespace std;

//Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

// Number of Pressure Transducers
const int NUM_PT = 3;

// Data Sending Interval Settings
unsigned long long delay_time = 200;
unsigned long long last_time = 0;

// ETHERNET CONNECTION
#define RO_PIN 16
#define DI_PIN 17
#define DE_RE_PIN 23

// HX711 LOAD CELL PINS
const int LC1_DOUT_PIN = 2;
const int LC1_SCK_PIN = 15;

// LOAD CELLS
const int NUM_LC = 1;
HX711 LC1;

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
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  // Load Cell Measurement
  LC1.begin(LC1_DOUT_PIN, LC1_SCK_PIN);
  //LC1.set_scale(LC1_calibration);
  //LC1.tare(); // reset load cell to 0
}

void loop() {
  if((millis() - last_time) > delay_time){
    float ptVals[NUM_PT];
    long lcVals[NUM_LC]; // since read() outputs long (heritage was using floats though)

    // Read Pressure Transducer Values
    for (int i = 0; i < NUM_PT; i++)
    {
      float raw = ads.readADC_SingleEnded(i);
      ptVals[i] = ads.computeVolts(raw);
    }

    // Read Load Cell Values
    if (LC1.is_ready()) {
      lcVals[0] = LC1.read(); // read() returns long
      //Serial.print("HX711 reading: ");
      //Serial.println(reading);
    } else {
      Serial.println("HX711 LC1 not found.");
    }
    
    //lcVals[0] = LC1.get_units(); //this is what heritage system used
    
    //  // Calibration for PTs (likely have to calibrate everytime you flow)
    ptVals[0] = ptVals[0] * 381.0376825 - 124.3444374;
    ptVals[1] = ptVals[1] * 381.2890799 - 120.337546;
    ptVals[2] = ptVals[2] * 380.6974487 - 114.0943018;
    // ptVals[3] = ptVals[3] * 420 - 259;

    //fake values to pass to GUI
    float pt4 = 0;
    float pt5 = 0;
    float pt6 = 0;  
    float lc1 = 0;  
    float lc2 = 0;  

    String storeStr = "Asensorvals pt1=" + String(ptVals[0]) + 
                  ",pt2=" + String(ptVals[1]) + 
                  ",pt3=" + String(ptVals[2]) + 
                  ",pt4=" + String(pt4) + 
                  ",pt5=" + String(pt5) + 
                  ",pt6=" + String(pt6) + 
                  ",lc1=" + String(lcVals[0]) + 
                  ",lc2=" + String(lc2) + 
                  "Z";
    Serial.println(storeStr);
    digitalWrite(DE_RE_PIN, HIGH);
    delay(50);
    rs485Serial.println(storeStr);
  }
}
