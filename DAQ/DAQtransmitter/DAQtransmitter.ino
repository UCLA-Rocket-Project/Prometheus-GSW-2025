//TODO: INCLUDE MICROSD AND LOAD CELL CODE FOR TRANSMISSION

#include <string>
#include <Adafruit_ADS1X15.h>
#include <HardwareSerial.h>
#include "SPI.h"
using namespace std;
#define ADS1_ADDRESS (0x49)

#include "HX711.h"
// HX711 #1
#define HX1_DOUT 2
#define HX1_CLK  15
// HX711 #2 (swapped as requested)
#define HX2_DOUT 33
#define HX2_CLK  32
HX711 scale1;
HX711 scale2;
float calibration_factor1 = -995.0;
float calibration_factor2 = -995.0;

//Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */
Adafruit_ADS1015 ads1; 
//Number of pressure transducers
const int NUM_PT = 6;
const int NUM_LC = 2
// Data Sending Interval Settings
unsigned long long delay_time = 200;
unsigned long long last_time = 0;

// ETHERNET CONNECTION
#define RO_PIN 16
#define DI_PIN 17
#define DE_RE_PIN 23

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
  // HX711 #1
  scale1.begin(HX1_DOUT, HX1_CLK);
  scale1.set_scale(calibration_factor1);
  scale1.tare();
  // HX711 #2
  scale2.begin(HX2_DOUT, HX2_CLK);
  scale2.set_scale(calibration_factor2);
  scale2.tare();
  Serial.println("HX711 #1 and #2 initialized and tared.");
  
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  if (!ads1.begin(ADS1_ADDRESS)) {
    Serial.println("Failed to initialize ADS1.");
    while (1);
  }
}
void loop() {
  if((millis() - last_time) > delay_time){
    float ptVals[NUM_PT];
    float lcVals[NUM_LC]

    // Read Pressure Transducer values
    for (int i = 0; i < 4; i++)
    {
      float raw = ads.readADC_SingleEnded(i);
      ptVals[i] = ads.computeVolts(raw);
    }

    for (int j = 4; j < 6; j++)
    {
      float raw = ads1.readADC_SingleEnded(i);
      ptVals[i] = ads1.computeVolts(raw);
    }
    //  // Calibration for PTs (likely have to calibrate everytime you flow)
    ptVals[0] = ptVals[0] * 381.0376825 - 124.3444374;
    ptVals[1] = ptVals[1] * 381.2890799 - 120.337546;
    ptVals[2] = ptVals[2] * 380.6974487 - 114.0943018;
    ptVals[3] = ptVals[3] * 1 - 0;
    ptVals[4] = ptVals[4] * 1 - 0;
    ptVals[5] = ptVals[5] * 1 - 0;

    lcVals[0] = scale1.get_units(5);
    lcVals[1] = scale2.get_units(5);
    // int pt3 = 0
    // int pt4 = 0
    // int pt5 = 0
    // int lc1 = 0
    // int lc2 = 0

    String storeStr = "Asensorvals pt1=" + String(ptVals[0]) + 
                  ",pt2=" + String(ptVals[1]) + 
                  ",pt3=" + String(ptVals[2]) + 
                  ",pt4=" + String(pt3) + 
                  ",pt5=" + String(pt4) + 
                  ",pt6=" + String(pt5) + 
                  ",lc1=" + String(lc1) + 
                  ",lc2=" + String(lc2) + 
                  "Z";
    Serial.println(storeStr);
    digitalWrite(DE_RE_PIN, HIGH);
    delay(50);
    rs485Serial.println(storeStr);
  }
}




