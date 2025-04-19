#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid = "GreenGuppy";
const char* password = "lebron123";

#define FILE_NAME_MAX_LENGTH 100
#define CSV_ENTRY_MAX_LENGTH 1024

char newFileName[FILE_NAME_MAX_LENGTH];

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// #define CLK 18
// #define CS 5
// #define MOSI 23
// #define MISO 19

// Body Tube
#define CLK 14
#define CS 15
#define MOSI 13
#define MISO 12

SPIClass spi;

// Init microSD card
void initSDCard(SPIClass& spi){

  if(!SD.begin(CS, spi)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

//  writeFile(SD, "/data0.csv", "HI,bye,there\n");
//  writeFile(SD, "/data1.csv", "HI,bye,there\n");
//  writeFile(SD, "/data2.csv", "HI,bye,there\n");
//  appendFile(SD, "/data.csv", "1,2,3\n");
  listDir(SD, "/", 0);
}

// Write to the SD card
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

// Delete file
void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\r\n", path);
  if(fs.remove(path)){
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

// Function that initializes wi-fi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
   Serial.printf("Listing directory: %s\n", dirname);
 
   File root = fs.open(dirname);
   if (!root) {
     Serial.println("Failed to open directory");
     return;
   }
   if (!root.isDirectory()) {
     Serial.println("Not a directory");
     return;
   }
 
   File file = root.openNextFile();
   while (file) {
     if (file.isDirectory()) {
       Serial.print("  DIR : ");
       Serial.println(file.name());
       if (levels) {
         listDir(fs, file.path(), levels - 1);
       }
     } else {
       Serial.print("  FILE: ");
       Serial.print(file.name());
       Serial.print("  SIZE: ");
       Serial.println(file.size());
     }
     file = root.openNextFile();
   }
 }

void setup() {
  Serial.begin(115200);
  initWiFi();

  spi.begin(CLK, MISO, MOSI, -1);
  initSDCard(spi);

  // Handle the root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "hey", "text/html");
  });

  // Handle the download button
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    char lastestFileName[FILE_NAME_MAX_LENGTH + 1];
    getLatestCSV(lastestFileName);
    request->send(SD, lastestFileName, String(), true);
  });

  // Handle the View Data button
  server.on("/view-data", HTTP_GET, [](AsyncWebServerRequest *request){
    char lastestFileName[FILE_NAME_MAX_LENGTH + 1];
    getLatestCSV(lastestFileName);
    request->send(SD, lastestFileName, "text/plain", false);
  });

  // Uncomment the following line if you need to serve more static files like CSS and javascript or favicon
  //server.serveStatic("/", SD, "/");

  server.begin();
}

void loop() {
}

void getLatestCSV(char* latestFileName) {
    int counter = 1;
    char candidateFileName[FILE_NAME_MAX_LENGTH + 1];
    char lastFoundFile[FILE_NAME_MAX_LENGTH + 1] = "ERROR"; // set as ERROR just in case

    while (true) {
        sprintf(candidateFileName, "/launch%d.txt", counter);
        //sprintf(candidateFileName, "/data%d.csv", counter);
        Serial.print("Checking file: ");
        Serial.println(candidateFileName);

        if (!SD.exists(candidateFileName)) {
            Serial.print("The latest file name was found to be: ");
            Serial.println(lastFoundFile);
            sprintf(latestFileName, lastFoundFile);
            return;
        }

        sprintf(lastFoundFile, candidateFileName);
        ++counter;
    }
}