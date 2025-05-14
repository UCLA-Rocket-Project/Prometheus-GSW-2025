#include <SPI.h>
#include <LoRa.h>
#define SCK_PIN 32
#define MISO_PIN 35
#define MOSI_PIN 25
#define CS_PIN 33
#define RESET_PIN 27
#define DIO0_PIN 14
#define TX_EN 26
#define RX_EN 12

struct GpsData {
  int32_t latitude;
  int32_t longitude;
  int32_t altitude; // gets height in mm above sea level
  int32_t heading;
};

struct ICMData {
  float accelX;
  float accelY;
  float accelZ;
  
  float gyroX;
  float gyroY;
  float gyroZ;
  
  float icmTemp;
};


struct BMPData {
  double bmpTemp;
  double pressure;
  double altitude;
};

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");
  pinMode(TX_EN, OUTPUT);
  pinMode(RX_EN, OUTPUT);
  digitalWrite(RX_EN, HIGH);
  digitalWrite(TX_EN, LOW);
  // Initialize SPI with custom pins (SCK, MISO, MOSI, CS)
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  // Tell LoRa library to use custom SPI
  LoRa.setSPI(SPI);
  // Set custom LoRa pins (CS, RESET, DIO0)
  LoRa.setPins(CS_PIN, RESET_PIN, DIO0_PIN);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  Serial.println("LoRa started successfully");
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();


  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    // read packet
    while (LoRa.available()) {
      char first = LoRa.read();
      char second = LoRa.read();
      
      if (first != 'A' || second != ' ') {
        continue;
      }

      struct GpsData gpsData = {};
      struct ICMData icmData = {};
      struct BMPData bmpData = {};
      unsigned long timeSinceStart;

      // read gps values
      uint8_t gpsDataBuffer[sizeof(GpsData)];
      LoRa.readBytes(gpsDataBuffer, sizeof(GpsData));
      memcpy(&gpsData, gpsDataBuffer, sizeof(GpsData));

      uint8_t icmDataBuffer[sizeof(ICMData)];
      LoRa.readBytes(icmDataBuffer, sizeof(ICMData));
      memcpy(&icmData, icmDataBuffer, sizeof(ICMData));

      uint8_t bmpDataBuffer[sizeof(BMPData)];
      LoRa.readBytes(bmpDataBuffer, sizeof(BMPData));
      memcpy(&bmpData, bmpDataBuffer, sizeof(BMPData));

      uint8_t timestampBuffer[sizeof(unsigned long)];
      LoRa.readBytes(timestampBuffer, sizeof(unsigned long));
      memcpy(&timeSinceStart, timestampBuffer, sizeof(unsigned long));

      char endOne = LoRa.read();
      char last = LoRa.read();

      if (endOne != ' ' || last != 'Z') {
        Serial.println("Invalid String Received");
        break;
      }

      char loraReceiveValue[1024];
      sprintf(loraReceiveValue, 
      "%3.5d,%3.5d,%5.8d,%3.5d," // gps data
      "%f,%f,%f," // accel
      "%f,%f,%f," // gyro
      "%f,%f,"
      "%lu\n", // bmp
      gpsData.latitude,
      gpsData.longitude,
      gpsData.altitude,
      gpsData.heading,
      icmData.accelX,
      icmData.accelY,
      icmData.accelZ,
      icmData.gyroX,
      icmData.gyroY,
      icmData.gyroZ,
      bmpData.pressure,
      bmpData.altitude,
      timeSinceStart
      );   
    Serial.println(loraReceiveValue);   
    }
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}