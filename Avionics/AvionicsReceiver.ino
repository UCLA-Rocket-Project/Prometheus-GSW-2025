#include <SPI.h>
#include <LoRa.h>

#define SCK_PIN 32
#define MISO_PIN 35
#define MOSI_PIN 25
#define CS_PIN 33
#define RESET_PIN 14
#define DIO0_PIN 27

#define STRING_MAX_LENGTH 1024
#define RSSI_READING_LENGTH 31

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver");

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

  LoRa.setTxPower(23);

  Serial.println("LoRa started successfully");
}
void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // read packet
    char receivedString[STRING_MAX_LENGTH + RSSI_READING_LENGTH + 1];
    size_t receivedStringIndex = 0;

    if (LoRa.available()) {
      receivedString[receivedStringIndex++] = (char)LoRa.read();
      
      // stop reading if the received string does not belong to us
      if (!receivedString[0]) {
        return;
      }

      while (LoRa.available() && receivedStringIndex < STRING_MAX_LENGTH) {
        // if the first character is not an 'A', the string does not belong to us
        receivedString[receivedStringIndex++] = (char)LoRa.read();
      }

      // check that the last 2 characters are Z\n, and add
      if (
        receivedStringIndex > 2 && 
        (receivedString[receivedStringIndex - 2] != 'Z' || receivedString[receivedStringIndex - 1] != '\n')
      ) {
        return;
      }

      // append the RSSI strength reading
      // replace the ' ' in ' Z\n' to append to the string

      receivedString[receivedStringIndex - 3] = '\0';
      char rssi_reading[RSSI_READING_LENGTH];
      sprintf(rssi_reading, ",%d Z\n", LoRa.packetRssi());

      strcat(receivedString, rssi_reading);
      Serial.print(receivedString);
    }
  }
}