#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid = "ILAY";
const char* password = "lebronpookie123";

#define FILE_NAME_MAX_LENGTH 100
#define CSV_ENTRY_MAX_LENGTH 1024

char newFileName[FILE_NAME_MAX_LENGTH];

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

File downloadFile;

// #define SD_HSCK   12  // Replace with your HSCK pin
// #define SD_HMISO  13  // Replace with your HMISO pin
// #define SD_HMOSI  11  // Replace with your HMOSI pin
// #define SD_CS_XTSD    6  // Replace with your CS_XTSD pin

// #define SD_HSCK 14
// #define SD_CS_XTSD 15
// #define SD_HMOSI 13
// #define SD_HMISO 12

// SD card
#define SD_CS_XTSD 4
#define SD_HSCK 18
#define SD_HMISO 13
#define SD_HMOSI 23

SPIClass spi;

// Init microSD card
void initSDCard(SPIClass& spi){

  if(!SD.begin(SD_CS_XTSD, spi)){
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

 writeFile(SD, "/data0.csv", "HI,bye,there\n");
 writeFile(SD, "/data1.csv", "HI,bye,there\n");
//  writeFile(SD, "/data2.csv", "HI,bye,there\n");
 appendFile(SD, "/data0.csv", "1,2,3\n");
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

void clearAllFiles(fs::FS &fs) {
  File root = fs.open("/", FILE_READ);

  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    Serial.printf("Deleting: %s\n", file.name());
    char fileName[30];
    snprintf(fileName, 30, "/%s", file.name());
    deleteFile(fs, fileName);
    file = root.openNextFile();
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


void listSDCardFiles(fs::FS &fs, char* inputBuffer) {
    sprintf(inputBuffer, "<html><body><ul>");
    
    File root = fs.open("/");

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
        char fileName[30];
        memset(fileName, 0, 30);
        sprintf(fileName, "<li>%s | %d</li>", file.name(), file.size());
        strcat(inputBuffer, fileName);
        Serial.println(fileName);
        file = root.openNextFile();
    }

    strcat(inputBuffer, "</ul></body></html>");
    Serial.println(inputBuffer);
}   

void setup() {
    Serial.begin(115200);
    delay(1000);
    initWiFi();

    spi.begin(SD_HSCK, SD_HMISO, SD_HMOSI, -1);
    initSDCard(spi);

    // Handle the root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "OK");
    });

    server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("sheesh")) {
            char sheeshParam[100];
            snprintf(sheeshParam, 100, request->getParam("sheesh")->value().c_str());
            Serial.printf("Received: %s\n", sheeshParam);
        }
        
        request->send(200, "text/plain", "WOOHOO");
    });

    server.on("/list-files", HTTP_GET, [](AsyncWebServerRequest *request) {
        char list[2048];
        listSDCardFiles(SD, list);
        char listToSend[2048];
        // sprintf(listToSend, "R\"rawliteral(%s)rawliteral\"", list);
        request->send(200, "text/html", list);
    });

    server.on("/download-chunked", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (!request->hasParam("fileName")) {
        request->send(200, "text/plain", "welp");
        return;
      }

      String filename;
      String filenameWithTxt = "/" + request->getParam("fileName")->value() + ".txt";
      String filenameWithCSV = "/" + request->getParam("fileName")->value() + ".csv";

      if (SD.exists(filenameWithTxt)) {
        filename = filenameWithTxt;
      }
      else if (SD.exists(filenameWithCSV)) {
        filename = filenameWithCSV;
      }
      else {
        request->send(404, "text/plain", "Provided file not found");
        return;
      }

      downloadFile = SD.open(filename, FILE_READ);
      size_t fileSize = downloadFile.size();
      String fileSizeString = String(fileSize);

      AsyncWebServerResponse* response = request->beginChunkedResponse("text/plain", [fileSize](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
        Serial.printf("%u / %u\n", index, fileSize);

        if (fileSize <= index) {
          Serial.println("finished");
          return 0;
        }
        
        const int chunkSize = min(maxLen, fileSize - index);
        char* fileReadBuffer = (char* )malloc(sizeof(char) * chunkSize);

        if (fileReadBuffer == NULL) {
          return 0;
        }

        downloadFile.readBytes(fileReadBuffer, chunkSize);
        memcpy(buffer, fileReadBuffer, chunkSize);
        free(fileReadBuffer);
        fileReadBuffer = NULL;

        Serial.printf("Sending %u\n", chunkSize);

        return chunkSize;
      });

      response->addHeader(asyncsrv::T_Cache_Control, "public,max-age=60");
      response->addHeader(asyncsrv::T_ETag, fileSizeString);

      request->send(response);
    });

    server.on("/delete-all-files", HTTP_GET, [](AsyncWebServerRequest* req) {
      if (!req->hasParam("password") || req->getParam("password")->value() != "juanmygoat") {
        req->send(400, "You dont have the permissions to do this");
        return;
      }

      clearAllFiles(SD);
      req->send(200, "All files deleted");
    });

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