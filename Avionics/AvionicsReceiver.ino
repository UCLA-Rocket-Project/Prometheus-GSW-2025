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

static char loraReceiveValue[512];

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
  // Try to parse a LoRa packet
  int packetSize = LoRa.parsePacket();
  yield();

  if (packetSize > 0) { // A LoRa packet was received (or the chip thought it received one)
    bool packetValidAndProcessed = false; // Flag to track if we should print RSSI for a valid packet
    Serial.print("Received potential packet, size: ");
    Serial.print(packetSize);

    // 1. Check for the "A " prefix
    // if (LoRa.available() >= 2) { // Must have at least 2 bytes for "A "
    //   char first = (char)LoRa.read();
    //   char second = (char)LoRa.read();

    //   if (first == 'A' && second == ' ') {
    //     // Prefix is correct. Now attempt to read the structured data and the final "Z".
    //     Serial.print(". Started with 'A '. ");

    //     struct GpsData gpsData = {};
    //     struct ICMData icmData = {};
    //     struct BMPData bmpData = {};
    //     unsigned long timeSinceStart;

    //     // Calculate the expected size of your data payload
    //     size_t expectedPayloadSize = sizeof(GpsData) +
    //                                  sizeof(ICMData) +
    //                                  sizeof(BMPData) +
    //                                  sizeof(unsigned long);
    //     size_t expectedEndMarkerSize = 1; // For the 'Z'

    //     // Check if enough bytes are available for the payload AND the final 'Z'
    //     if (LoRa.available() >= (expectedPayloadSize + expectedEndMarkerSize)) {
    //       // Read the structured data
    //       uint8_t gpsDataBuffer[sizeof(GpsData)];
    //       LoRa.readBytes(gpsDataBuffer, sizeof(GpsData));
    //       memcpy(&gpsData, gpsDataBuffer, sizeof(GpsData));

    //       uint8_t icmDataBuffer[sizeof(ICMData)];
    //       LoRa.readBytes(icmDataBuffer, sizeof(ICMData));
    //       memcpy(&icmData, icmDataBuffer, sizeof(ICMData));

    //       uint8_t bmpDataBuffer[sizeof(BMPData)];
    //       LoRa.readBytes(bmpDataBuffer, sizeof(BMPData));
    //       memcpy(&bmpData, bmpDataBuffer, sizeof(BMPData));

    //       uint8_t timestampBuffer[sizeof(unsigned long)];
    //       LoRa.readBytes(timestampBuffer, sizeof(unsigned long));
    //       memcpy(&timeSinceStart, timestampBuffer, sizeof(unsigned long));

    //       // Read the expected 'Z' end marker
    //       char last = (char)LoRa.read();

    //       if (last == 'Z') {
    //         // Packet is fully valid from start to end!
    //         Serial.print("Ended with 'Z'. Packet valid. ");
    //         packetValidAndProcessed = true; // Mark for RSSI printing

    //         snprintf(loraReceiveValue, 512,
    //                 "%ld,%ld,%ld,%ld,"     // GPS data (latitude, longitude, altitude, heading) as long integers
    //                 "%.2f,%.2f,%.2f,"     // Accel data (X, Y, Z) as floats with 2 decimal places
    //                 "%.2f,%.2f,%.2f,"     // Gyro data (X, Y, Z) as floats with 2 decimal places
    //                 "%.2f,%.2f,"          // BMP data (pressure, altitude) as floats with 2 decimal places
    //                 "%lu",                // timeSinceStart (unsigned long)
    //                 (long)gpsData.latitude,   // Explicitly cast if they are not already long
    //                 (long)gpsData.longitude,
    //                 (long)gpsData.altitude,
    //                 (long)gpsData.heading,
    //                 icmData.accelX,  
    //                 icmData.accelY,
    //                 icmData.accelZ,
    //                 icmData.gyroX,
    //                 icmData.gyroY,
    //                 icmData.gyroZ,
    //                 bmpData.pressure,
    //                 bmpData.altitude, 
    //                 timeSinceStart);
    //         Serial.print("Data Received: ");
    //         // Serial.print(loraReceiveValue); // Use print here

    //       } else {
    //         Serial.print("Expected 'Z' at end, but got '");
    //         Serial.print(last);
    //         Serial.print("'. Packet corrupt. ");
    //       }
    //     } else {
    //       Serial.print("Started with 'A ' but not enough data for full payload and 'Z'. Packet incomplete. ");
    //     }
    //   } else {
    //     Serial.print(". Did not start with 'A '. Packet corrupt. ");
    //   }
    // } else if (packetSize > 0) {
    //     Serial.print(". Packet too small for 'A ' marker. Size: ");
    //     Serial.print(packetSize);
    // }

    // If we attempted to process a packet (i.e. packetSize > 0), print RSSI.
    // You might only want to print RSSI for packetValidAndProcessed = true.
    // If so, wrap the next two lines in: if (packetValidAndProcessed) { ... }
    Serial.print(" RSSI: "); // Note the leading space for formatting if loraReceiveValue was printed
    Serial.println(LoRa.packetRssi());

    // Ensure any remaining bytes from this LoRa.parsePacket() event are flushed.
    // This is important if packetSize was larger than expected, or if we bailed early.
    int flushedCount = 0;
    while (LoRa.available() > 0) {
      Serial.print((char)LoRa.read());
      flushedCount++;
    }

    if (flushedCount > 0) {
      Serial.print("Flushed ");
      Serial.print(flushedCount);
      Serial.println(" unexpected remaining byte(s) from LoRa buffer.");
    }

  } else if (packetSize == 0) {
    yield();
  }
}