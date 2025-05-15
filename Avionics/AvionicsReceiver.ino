#include <SPI.h>
#include <LoRa.h>

// Define LoRa chip pins
#define SCK_PIN 32
#define MISO_PIN 35
#define MOSI_PIN 25
#define CS_PIN 33
#define RESET_PIN 27
#define DIO0_PIN 14 // Must be connected for onReceive to work

// Define RF switch pins (if used)
#define TX_EN 26
#define RX_EN 12

unsigned long last_start_time = 0;

// Data structures for received packet
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

// Buffer for formatting the received data string for printing
static char loraReceiveValue[512];

// --- Globals for ISR and Loop communication ---
// Volatile because it's modified by an ISR and read by the main loop
volatile bool packetAvailable = false;
volatile int incomingPacketSize = 0;
// It's generally better not to read RSSI in the ISR if it involves SPI after the main packet read.
// We will read it in the main loop after confirming packet reception.
// volatile int incomingPacketRssi = 0;


// --- Function Prototypes ---
void onPacketReceived(int packetSize); // This is our LoRa onReceive callback
void processAndPrintPacket(int packetSize); // Processes and prints the packet data

void setup() {
  Serial.begin(115200);
  // It's often better to remove `while(!Serial);` for headless operation,
  // but fine for debugging if you always connect the monitor quickly.
  // while (!Serial); 
  Serial.println("LoRa Receiver with onReceive Callback");

  // Initialize RF switch pins (if your module has them)
  pinMode(TX_EN, OUTPUT);
  pinMode(RX_EN, OUTPUT);
  digitalWrite(RX_EN, HIGH); // Enable Receiver
  digitalWrite(TX_EN, LOW);  // Disable Transmitter

  // Initialize SPI for LoRa
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  LoRa.setSPI(SPI);
  LoRa.setPins(CS_PIN, RESET_PIN, DIO0_PIN);

  // Start LoRa module
  if (!LoRa.begin(915E6)) { // Using 915 MHz
    Serial.println("Starting LoRa failed!");
    while (1); // Halt on failure
  }

  // Set LoRa parameters - adjust as needed
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(250E3); // Matching your previous code's last value
  // LoRa.enableCrc(); // Good practice to enable CRC for data integrity

  Serial.println("LoRa initialized successfully.");

  // Register the callback function to be called when a packet is received
  // The onPacketReceived function will be executed by the LoRa library,
  // likely in an interrupt context or a similar high-priority context.
  LoRa.onReceive(onPacketReceived);

  // Put LoRa module into continuous receive mode
  // This is crucial to start listening.
  LoRa.receive();
}

// This function is the callback executed by the LoRa library when a packet arrives.
// It should be KEPT VERY SHORT AND FAST.
// For ESP32, it's good practice to mark ISR functions with IRAM_ATTR
// to ensure they are loaded into RAM for faster execution and to avoid
// issues with flash cache during interrupt handling.
void IRAM_ATTR onPacketReceived(int packetSize) {
  if (packetSize == 0) {
    return; // No packet or an error, ignore.
  }

  // Check if a previous packet is still pending processing by the main loop.
  // This is a simple way to handle potential overruns if the main loop is slow.
  if (packetAvailable) {
    // A packet was received, but the main loop hasn't processed the previous one yet.
    // This new packet will be missed by this simple logic.
    // More advanced handling might use a queue.
    Serial.println("OVF_ISR"); // Overrun detected in ISR
    return; 
  }

  // Store the packet size and set the flag for the main loop.
  // The actual reading of the packet data will happen in the main loop
  // to keep this ISR short.
  incomingPacketSize = packetSize;
  packetAvailable = true;
  
  // DO NOT do Serial.print or other long operations here.
  // DO NOT do LoRa.read() or LoRa.readBytes() here if you want to keep the ISR absolutely minimal.
  // The LoRa library has already parsed the packet to get the size.
  // The main loop will now be responsible for reading the data.
}

void loop() {
  // Check if the ISR has signaled that a packet is available
  unsigned long current_time = millis();

    //   if no signal for more than 10 seconds, dump regs and freeze
  if (current_time - last_start_time > 10000) {
    Serial.println("No receipt for the last 10 seconds");
    LoRa.dumpRegisters(Serial);
    while (1) {}
  }

  if (packetAvailable) {
    // Atomically reset the flag. For a single boolean, direct assignment is usually fine.
    // For more complex scenarios or multi-core, consider critical sections or atomic operations.
    noInterrupts(); // Start critical section
    packetAvailable = false;
    int currentPacketSize = incomingPacketSize; // Copy volatile to local
    interrupts();   // End critical section

    // Call the function to actually read, process, and print the packet
    processAndPrintPacket(currentPacketSize);

    // After processing, put LoRa module back into continuous receive mode.
    // This is essential to listen for the next packet.
    LoRa.receive();
  }

  // You can do other non-blocking tasks in the loop here.
  yield(); // Good practice on ESP32 to allow other tasks (like WiFi) to run.

  last_start_time = current_time;
}

// This function reads, parses, and prints the packet data.
// It's called from the main loop, so Serial.print and other operations are safe here.
void processAndPrintPacket(int packetSize) {
//   int bufferAddress = LoRa.readRegister(0x10);

//   // if buffer has overrun, perform a software reset
//   if (bufferAddress == 0xff) {
//     ESP.restart();
//   }

  Serial.print("Processing packet, size: ");
  Serial.print(packetSize);

  bool packetIsValid = false; // Flag to track if the packet structure is valid

  // 1. Check for the "A " prefix
  // LoRa.available() should match packetSize at this point if no other reads happened.
  if (LoRa.available() >= 2) {
    char first = (char)LoRa.read(); // Read first byte
    char second = (char)LoRa.read(); // Read second byte

    if (first == 'A' && second == ' ') {
      Serial.print(". Prefix 'A ' OK. ");
      // Prefix is correct. Now attempt to read the structured data and the final "Z".
      struct GpsData gpsData = {};
      struct ICMData icmData = {};
      struct BMPData bmpData = {};
      unsigned long timeSinceStart = 0;

      // Calculate the expected size of your data payload
      size_t expectedPayloadSize = sizeof(GpsData) +
                                   sizeof(ICMData) +
                                   sizeof(BMPData) +
                                   sizeof(unsigned long);
      size_t expectedEndMarkerSize = 1; // For the 'Z'

      // Check if enough bytes are available for the payload AND the final 'Z'
      // LoRa.available() will have decreased by 2 due to reading the prefix.
      if (LoRa.available() >= (expectedPayloadSize + expectedEndMarkerSize)) {
        // Read the structured data
        LoRa.readBytes((uint8_t*)&gpsData, sizeof(GpsData));
        LoRa.readBytes((uint8_t*)&icmData, sizeof(ICMData));
        LoRa.readBytes((uint8_t*)&bmpData, sizeof(BMPData));
        LoRa.readBytes((uint8_t*)&timeSinceStart, sizeof(unsigned long));

        // Read the expected 'Z' end marker
        char secondLast = (char)LoRa.read();
        char last = (char)LoRa.read();

        if (last == 'Z') {
          Serial.print("Footer 'Z' OK. Packet valid. ");
          packetIsValid = true;

          // Format the data for printing
          snprintf(loraReceiveValue, sizeof(loraReceiveValue), // Use sizeof for safety
                  "%ld,%ld,%ld,%ld,"     // GPS data
                  "%.2f,%.2f,%.2f,"     // Accel data
                  "%.2f,%.2f,%.2f,"     // Gyro data
                  "%.2f,%.2f,"          // BMP data
                  "%lu",                // timeSinceStart
                  (long)gpsData.latitude, (long)gpsData.longitude, (long)gpsData.altitude, (long)gpsData.heading,
                  icmData.accelX, icmData.accelY, icmData.accelZ,
                  icmData.gyroX, icmData.gyroY, icmData.gyroZ,
                  bmpData.pressure, bmpData.altitude,
                  timeSinceStart);
          Serial.print("Data: ");
          Serial.print(loraReceiveValue); // Print the formatted data

        } else {
          Serial.print("Expected 'Z' at end, but got '");
          Serial.print(last);
          Serial.print("'. Corrupt. ");
        }
      } else {
        Serial.print("Prefix 'A ' OK, but not enough data for payload & 'Z'. Incomplete. ");
      }
    } else {
      Serial.print(". Did not start with 'A '. Corrupt. ");
    }
  } else if (packetSize > 0) { // packetSize > 0 but LoRa.available() < 2
      Serial.print(". Too small for 'A ' marker. ");
  } else { // packetSize was 0, should have been caught by ISR, but defensive check
      Serial.print(". No data or zero size packet. ");
  }

  // Print RSSI. It's best to read RSSI after confirming a packet was processed.
  // The LoRa.packetRssi() call should be made after LoRa.parsePacket() (which is done internally by the library before onReceive)
  // and before the next LoRa.receive() call if possible, or at least while the context of the last packet is still valid.
  Serial.print(" RSSI: ");
  Serial.print(LoRa.packetRssi());
  Serial.println(); // End the line for this packet's log

  // Flush any unexpected remaining bytes from the LoRa FIFO.
  // This is crucial if packetSize reported by the chip was larger than what your protocol expected,
  // or if your parsing logic bailed out early.
  int flushedCount = 0;
  while (LoRa.available() > 0) {
    (void)LoRa.read(); // Read and discard
    flushedCount++;
  }
  if (flushedCount > 0) {
    Serial.print("Warning: Flushed ");
    Serial.print(flushedCount);
    Serial.println(" unexpected remaining byte(s) from LoRa FIFO.");
  }
}
