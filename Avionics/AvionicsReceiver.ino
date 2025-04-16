#include <SPI.h>
#include <LoRa.h>

#define SCK_PIN 32
#define MISO_PIN 35
#define MOSI_PIN 25
#define CS_PIN 33
#define RESET_PIN 14
#define DIO0_PIN 27

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");

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

  Serial.println("LoRa started successfully");
}
void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    Serial.println();
  }
}