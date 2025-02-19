//NOTE: 
// your wifi name should not have any apostrophes or spaces
// restarting the esp32 often works well for reconnection lmao

//TODO:
// - figure out how to send messages to python - MQTT?
// - dump message to show all available networks (maybe idk not a high priority)

#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "username"; //this name should not have spaces or apostrophes
const char* password = "password";
const char *udpAddress = "127.0.0.1"; // Your computer's IP address
const int udpPort = 80;  // Port to send the packet to
WiFiUDP udp;

void setup(){
    Serial.begin(115200);
    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.println("Still connecting...");
        delay(1000);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("\nStarting connection to server...");

    udp.begin(udpPort);
}


void loop() {
 //todo @euan maybe having code to dump out available networks on command is useful

 if (WiFi.status() == WL_CONNECTED){
 // Example message to send
  String message = "Hello from ESP32!";
  
  // Send UDP packet
  udp.beginPacket(udpAddress, udpPort);
  udp.print(message);
  udp.endPacket();
  Serial.println("sent message to udp port!");

  // Wait for a bit before sending the next packet
  delay(1000);
 }  else {
    static unsigned long lastReconnectAttempt = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastReconnectAttempt > 5000) {  // Try reconnecting every 5 seconds
      Serial.println("Disconnected from WiFi! Attempting to reconnect...");
      WiFi.reconnect();  // Try reconnecting
      lastReconnectAttempt = currentMillis;  // Update the last attempt time
    }
  }
}
