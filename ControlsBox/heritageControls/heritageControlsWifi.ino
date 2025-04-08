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
#define purge 5
#define mpv 4
#define ignite 15 //15
#define abortSiren 23
#define abortValve 13  //13
unsigned long long delay_time = 250;
unsigned long long last_time = 0;

// Replace the next variables with your SSID/Password combination
const char* ssid = "GreenGuppy";
const char* password = "lebron123";
// Add your MQTT Broker IP address, example:
const char* mqtt_server = "172.20.10.10";
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
    const short PURGE_SWITCH = 4;
    const short FILL_SWITCH = 5;
    const short ABORT_SIREN_SWITCH = 9;
    const short DUMP_SWITCH = 6;
    const short VENT_SWITCH = 2;
    const short QD_SWITCH = 1;
    const short IGNITE_SWITCH = 3;
    const short MPV_SWITCH = 8;
    const short OUTLET_SWITCH = 7;
    const short ABORT_VALVE_SWITCH = 0;

    if(message[FILL_SWITCH] == ACTUATED){
        Serial.println("FILL Command: ON");
        digitalWrite(fill, LOW);
    }
    if(message[FILL_SWITCH] == '0'){
        Serial.println("FILL Command: OFF");
        digitalWrite(fill, HIGH);
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
        digitalWrite(dump, LOW);
    }
    if(message[DUMP_SWITCH] == '0'){
        Serial.println("DUMP Command: OFF");
        digitalWrite(dump, HIGH);
    }

    if(message[VENT_SWITCH] == ACTUATED){
        Serial.println("VENT Command: ON");
        digitalWrite(vent, LOW);
    }
    if(message[VENT_SWITCH] == '0'){
        Serial.println("VENT Command: OFF");
        digitalWrite(vent, HIGH);
    }

    if(message[PURGE_SWITCH] == ACTUATED){
        Serial.println("PURGE Command: ON");
        digitalWrite(purge, LOW);
    }
    if(message[PURGE_SWITCH] == '0'){
        Serial.println("PURGE Command: OFF");
        digitalWrite(purge, HIGH);
    }

    if(message[QD_SWITCH] == ACTUATED){
        Serial.println("QD Command: ON");
        digitalWrite(qd, LOW);
    }
    if(message[QD_SWITCH] == '0'){
        Serial.println("QD Command: OFF");
        digitalWrite(qd, HIGH);
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
        digitalWrite(mpv, LOW);
    }
    if(message[MPV_SWITCH] == '0'){
        Serial.println("MPV Command: OFF");
        digitalWrite(mpv, HIGH);
    }

    if(message[OUTLET_SWITCH] == ACTUATED){
        Serial.println("OUTLET Command: ON");
        digitalWrite(outlet, LOW);
    }
    if(message[OUTLET_SWITCH] == '0'){
        Serial.println("OUTLET Command: OFF");
        digitalWrite(outlet, HIGH);
    }

    if(message[ABORT_VALVE_SWITCH] == ACTUATED) {
        Serial.println("ABORT VALVE Command: ON -- EMERGENCY SHUTDOWN");
        digitalWrite(ignite, LOW);
        digitalWrite(fill, HIGH);
        digitalWrite(vent, HIGH);
        digitalWrite(dump, HIGH);
        digitalWrite(qd, HIGH);
        digitalWrite(mpv, HIGH);
        digitalWrite(purge, HIGH);
        digitalWrite(abortValve, HIGH);
    }

    if(message[ABORT_VALVE_SWITCH] == '0'){
        Serial.println("ABORT VALVE Command: OFF");
        digitalWrite(abortValve, LOW);
    }

    last_time = millis();
}


void connect_client() {
  while (!client.connected()) {
    if (client.connect("ESP32")) {
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
  pinMode(purge, OUTPUT);
  pinMode(outlet, OUTPUT);
  pinMode(abortValve, OUTPUT);

  digitalWrite(abortSiren, LOW);//off
  digitalWrite(ignite, LOW);//off
  digitalWrite(fill, HIGH);//closed
  digitalWrite(vent, HIGH);//open
  digitalWrite(dump, HIGH);//open
  digitalWrite(qd, HIGH);//open
  digitalWrite(mpv, HIGH);
  digitalWrite(purge, HIGH);//closed
  digitalWrite(outlet, HIGH);//closed
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
  while (!client.connected() && millis() - last_time > 1000){
    connect_client();

    Serial.println("Unavailable...");
     if (millis() - last_time > 10000){
       digitalWrite(abortSiren, HIGH);
    }
    if (millis() - last_time > 20000){
      Serial.println("Aborted...");
      digitalWrite(ignite, LOW);
      digitalWrite(fill, HIGH);
      digitalWrite(vent, HIGH);
      digitalWrite(dump, HIGH);
      digitalWrite(qd, HIGH);
      digitalWrite(mpv, HIGH);
      digitalWrite(purge, HIGH);
      digitalWrite(ignite, LOW);
      digitalWrite(abortValve, LOW);
    }
  }

  client.loop();
}