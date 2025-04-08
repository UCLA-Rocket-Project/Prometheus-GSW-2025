// Changelog: 
// 4/8 Let the receiver run over wifi // test on terminal with: mosquitto_pub -h 172.20.10.10  -t esp32/output -m "hello"

#include <WiFi.h>
#include <PubSubClient.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "GreenGuppy";
const char* password = "lebron123";
// Add your MQTT Broker IP address, example:
const char* mqtt_server = "172.20.10.10";
// run `ipconfig getifaddr en0` in your macbook terminal
const char* DAQ_TOPIC = "esp32/output";

WiFiClient espClient;
PubSubClient client(espClient);

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

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void connect_client() {
  while (!client.connected()) {
    if (client.connect("ESP32")) {
        client.subscribe(DAQ_TOPIC);
        Serial.println("Reconnected to MQTT broker");
    } else {
        Serial.println("Failed to reconnect to MQTT broker");
        delay(5000);  // Wait before retrying
    }
  }

}
  
void setup() {
    Serial.begin(9600);
    delay(1000);

    //wifi code
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    setup_wifi();

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    connect_client();
}

void loop() {
  connect_client();
  client.loop();
}