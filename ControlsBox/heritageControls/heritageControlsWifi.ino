#include <WiFi.h>
#include <PubSubClient.h>

#define RO_PIN 16
#define DI_PIN 17
#define outlet 2 //32
//#define SER_BUF_SIZE 1024
#define fill 22
#define dump 21
#define vent 19
#define qd 18
#define mpv 5
#define ignite 4 //15
#define abortSiren 23
#define abortValve 13  //13
unsigned long long delay_time = 250;
unsigned long long last_time = 0;

// Replace the next variables with your SSID/Password combination
const char* ssid = "ILAY";
const char* password = "lebronpookie123";
// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.0.102";
// run `ipconfig getifaddr en0` in your macbook terminal
const char* SWITCHBOX_TOPIC = "switchbox/commands";

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

    Serial.println("received!");
    char received = (char)payload[0];

    label:
        String message;
        if (received == 'A'){
            for (int i = 1; i < length; i++) {
                received = (char)payload[i];
                if ((i != length - 1 && received != '1' && received != '0') || (i == length - 1 && received != 'Z')){
                    Serial.println("Invalid Message: " + String(received));
                    goto label;
                }
                message += received;
            }
            Serial.print("Received: ");
            Serial.println(message);
        }

    const char ACTUATED = '1';

    const short ABORT_VALVE_SWITCH = 0;
    const short QD_SWITCH = 1;
    const short VENT_SWITCH = 2;
    const short IGNITE_SWITCH = 3;
    const short PURGE_SWITCH = 4;
    const short FILL_SWITCH = 5;
    const short DUMP_SWITCH = 6;
    const short HEATPAD_SWITCH = 7;
    const short MPV_SWITCH = 8;
    const short ABORT_SIREN_SWITCH = 9;




    if(message[FILL_SWITCH] == ACTUATED){
        Serial.println("FILL Command: ON");
        digitalWrite(fill, HIGH);
    }
    if(message[FILL_SWITCH] == '0'){
        Serial.println("FILL Command: OFF");
        digitalWrite(fill, LOW);
    }

    if(message[ABORT_SIREN_SWITCH] == ACTUATED){
        Serial.println("ABORT SIREN Command: ON");
        digitalWrite(abortSiren, HIGH);
    }
    if(message[ABORT_SIREN_SWITCH] == '0'){
        Serial.println("ABORT SIREN Command: OFF");
        digitalWrite(abortSiren, LOW);
    }

    if(message[DUMP_SWITCH] == ACTUATED){
        Serial.println("DUMP Command: ON");
        digitalWrite(dump, HIGH);
    }
    if(message[DUMP_SWITCH] == '0'){
        Serial.println("DUMP Command: OFF");
        digitalWrite(dump, LOW);
    }

    if(message[VENT_SWITCH] == ACTUATED){
        Serial.println("VENT Command: ON");
        digitalWrite(vent, HIGH);
    }
    if(message[VENT_SWITCH] == '0'){
        Serial.println("VENT Command: OFF");
        digitalWrite(vent, LOW);
    }

    // if(message[PURGE_SWITCH] == ACTUATED){
    //     Serial.println("PURGE Command: ON");
    //     digitalWrite(purge, HIGH);
    // }
    // if(message[PURGE_SWITCH] == '0'){
    //     Serial.println("PURGE Command: OFF");
    //     digitalWrite(purge, LOW);
    // }

    if(message[QD_SWITCH] == ACTUATED){
        Serial.println("QD Command: ON");
        digitalWrite(qd, HIGH);
    }
    if(message[QD_SWITCH] == '0'){
        Serial.println("QD Command: OFF");
        digitalWrite(qd, LOW);
    }

    if(message[IGNITE_SWITCH] == ACTUATED){
        Serial.println("IGNITE Command: ON");
        digitalWrite(ignite, HIGH);
    }
    if(message[IGNITE_SWITCH] == '0'){
        Serial.println("IGNITE Command: OFF");
        digitalWrite(ignite, LOW);
    }

    if(message[MPV_SWITCH] == ACTUATED){
        Serial.println("MPV Command: ON");
        digitalWrite(mpv, HIGH);
    }
    if(message[MPV_SWITCH] == '0'){
        Serial.println("MPV Command: OFF");
        digitalWrite(mpv, LOW);
    }

    // if(message[OUTLET_SWITCH] == ACTUATED){
    //     Serial.println("OUTLET Command: ON");
    //     digitalWrite(outlet, HIGH);
    // }
    // if(message[OUTLET_SWITCH] == '0'){
    //     Serial.println("OUTLET Command: OFF");
    //     digitalWrite(outlet, LOW);
    // }

    if(message[ABORT_VALVE_SWITCH] == ACTUATED) {
        Serial.println("ABORT VALVE Command: ON -- EMERGENCY SHUTDOWN");
        digitalWrite(ignite, LOW);
        digitalWrite(fill, LOW);
        digitalWrite(vent, LOW);
        digitalWrite(dump, LOW);
        digitalWrite(qd, LOW);
        digitalWrite(mpv, LOW);
        // digitalWrite(purge, LOW);
        digitalWrite(abortValve, LOW);
    }

    if(message[ABORT_VALVE_SWITCH] == '0'){
        Serial.println("ABORT VALVE Command: OFF");
        digitalWrite(abortValve, LOW);
    }

    last_time = millis();
}


void connect_client() {
  while (!client.connected()) {
    if (client.connect("ESP32_SWITCHBOX")) {
        client.subscribe(SWITCHBOX_TOPIC);
        Serial.println("Reconnected to MQTT broker");
    } else {
        Serial.println("Failed to reconnect to MQTT broker");
        delay(5000);  // Wait before retrying
    }
  }

}

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("HII");

  pinMode(abortSiren, OUTPUT);
  pinMode(ignite, OUTPUT);
  pinMode(fill, OUTPUT);
  pinMode(vent, OUTPUT);
  pinMode(dump, OUTPUT);
  pinMode(qd, OUTPUT);
  pinMode(mpv, OUTPUT);
  // pinMode(purge, OUTPUT);
  pinMode(outlet, OUTPUT);
  pinMode(abortValve, OUTPUT);

  digitalWrite(abortSiren, LOW);//off
  digitalWrite(ignite, LOW);//off
  digitalWrite(fill, LOW);//closed
  digitalWrite(vent, LOW);//open
  digitalWrite(dump, LOW);//open
  digitalWrite(qd, LOW);//open
  digitalWrite(mpv, LOW);
  // digitalWrite(purge, LOW);//closed
  digitalWrite(outlet, LOW);//closed
  digitalWrite(abortValve, LOW);//closed

  //wifi code
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  setup_wifi();

  // connect to MQTT server
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  connect_client();

  last_time = millis();

  Serial.println("done with setup");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Network disconnection detected");

      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      delay(500);
      unsigned long long disconnect_start = millis();

      while (WiFi.status() != WL_CONNECTED) {
        if((millis() - disconnect_start) > 20000) {
          Serial.println("Disconnected for 20s EMERGENCY SHUTDOWN");
          digitalWrite(ignite, LOW);
          digitalWrite(fill, LOW);
          digitalWrite(vent, LOW);
          digitalWrite(dump, LOW);
          digitalWrite(qd, LOW);
          digitalWrite(mpv, LOW);
          // digitalWrite(purge, LOW);
          digitalWrite(abortValve, LOW);
        }
        Serial.println("Reconnecting to Wifi...");
        delay(250);
      }
    }
  
  if (!client.connected() && millis() - last_time > 1000){
    connect_client();
  }

  client.loop();
}