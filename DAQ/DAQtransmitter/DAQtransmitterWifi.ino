//TODO: INCLUDE MICROSD AND LOAD CELL CODE FOR TRANSMISSION
//Changelog:
// 4/8: Added wifi 

#include <WiFi.h>
#include <PubSubClient.h>

#include <string>
#include <Adafruit_ADS1X15.h>
#include <HardwareSerial.h>
#include "SPI.h"
#include <HX711.h>
// HX711 #1
#define HX1_DOUT 2
#define HX1_CLK  15
// HX711 #2 (swapped as requested)
#define HX2_DOUT 33
#define HX2_CLK  32

#define ADS1_ADDRESS (0x49)

using namespace std;

// Replace the next variables with your SSID/Password combination
const char* ssid = "ILAY";
const char* password = "lebronpookie123";
// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.0.103";
// run `ipconfig getifaddr en0` in your macbook terminal
WiFiClient espClient;
PubSubClient client(espClient);

HX711 scale1;
HX711 scale2;
float calibration_factor1 = -996.0;

//Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */
Adafruit_ADS1015 ads1; 
//Number of pressure transducers
const int NUM_PT = 6;
const int NUM_LC = 2;
// Data Sending Interval Settings
unsigned long long delay_time = 200;
unsigned long long last_time = 0;

// ETHERNET CONNECTION
#define RO_PIN 16
#define DI_PIN 17
#define DE_RE_PIN 23

// HardwareSerial rs485Serial(2);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("still connecting...");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  //Ethernet Connection
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, HIGH);

  // put your setup code here, to run once:
  // rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);
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
  scale1.set_scale(-995.0);
  scale1.tare();
  // HX711 #2
  scale2.begin(HX2_DOUT, HX2_CLK);
  scale2.set_scale(-995.0);
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

  //wifi code
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}
void loop() {
  // retry loop in case internet connection fails
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Network disconnection detected");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    delay(500);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to Wifi...");
      delay(500);
    }
  }

  if((millis() - last_time) > delay_time) {
    float ptVals[NUM_PT];
    float lcVals[NUM_LC];

    // Read Pressure Transducer values
    for (int i = 0; i < 4; i++)
    {
      //ptVals[i] = 5.0;
      float raw = ads.readADC_SingleEnded(i);
      ptVals[i] = ads.computeVolts(raw);
    }

    for (int j = 4; j < 6; j++)
    {
      //ptVals[j] = 5.0;
      float raw = ads1.readADC_SingleEnded(j-4);
      ptVals[j] = ads1.computeVolts(raw);
    }

    //   383.7480065  -119.6409757
    // 381.2315195  -125.5011544
    // 381.4313789  -108.0331038
    // 388.3539547  -107.9081708
    // 377.9846196  -117.9140663
    //  // Calibration for PTs (likely have to calibrate everytime you flow)
    ptVals[0] = ptVals[0] * 383.7480065 -119.6409757;
    ptVals[1] = ptVals[1] * 381.2315195 -125.5011544;
    ptVals[2] = ptVals[2] * 381.4313789 -108.0331038;
    ptVals[3] = ptVals[3] * 388.3539547 -107.9081708;
    ptVals[4] = ptVals[4] * 377.9846196 -117.9140663;
    ptVals[5] = ptVals[5] * 1 - 0;

    // lcVals[0] = 5.0;
    // lcVals[1] = 5.0;
    lcVals[0] = scale1.get_units();
    lcVals[1] = scale2.get_units();
    // int pt3 = 0
    // int pt4 = 0
    // int pt5 = 0
    // int lc1 = 0
    // int lc2 = 0

    String storeStr = "Asensorvals pt1=" + String(ptVals[0]) + 
                  ",pt2=" + String(ptVals[1]) + 
                  ",pt3=" + String(ptVals[2]) + 
                  ",pt4=" + String(ptVals[3]) + 
                  ",pt5=" + String(ptVals[4]) + 
                  ",pt6=" + String(ptVals[5]) + 
                  ",lc1=" + String(lcVals[0]) + 
                  ",lc2=" + String(lcVals[1]) +
                  ",timestamp=" + String(millis())+ 
                  "Z";
                  
    while (!client.connected()) {
      if (client.connect("ESP32")) {
          Serial.println("Reconnected to MQTT broker");
      } else {
          Serial.println("Failed to reconnect to MQTT broker");
          delay(5000);  // Wait before retrying
      }
    }
    client.publish("esp32/output", storeStr.c_str());

    Serial.println(storeStr);
    digitalWrite(DE_RE_PIN, HIGH);
    delay(50);
    // rs485Serial.println(storeStr);
  }
}