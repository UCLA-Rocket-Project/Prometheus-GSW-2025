#include <Arduino.h>
#include <string>
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_NeoPixel.h>

// LED Configuration
#define LED_PIN   21
#define LED_COUNT 69
#define BRIGHTNESS 0.2 * 255 // Set BRIGHTNESS to about 1/5 (max = 255)

// Declare NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

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

const int STATE_LENGTH = 10;
int state[STATE_LENGTH] = {0,0,0,0,0,0,0,0,0,0};
int prevState[STATE_LENGTH] = {0,0,0,0,0,0,0,0,0,0};

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

void connect_client() {
  while (!client.connected()) {
    if (client.connect("ESP32")) {
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
  
  //wifi code
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  setup_wifi();

  // connect to MQTT server
  client.setServer(mqtt_server, 1883);
  connect_client();

  Serial.println("Wifi Setup Complete");

  // Initialize LED strip
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);

  Serial.println("LED Setup Complete");

  last_time = millis();
}

void executeCommand(String command) {
    // Convert to lowercase for case-insensitive comparison
    command.toLowerCase();
    if(command == "fill") {
      if(state[0] == 0) {
        state[0] = 1;
      } else {
        state[0] = 0;
      }
    } else if (command == "siren") {
      if(state[2] == 0) {
        state[2] = 1;
      } else {
        state[2] = 0;
      }
    } else if (command == "dump") {
      if(state[3] == 0) {
        state[3] = 1;
      } else {
        state[3] = 0;
      }
    } else if (command == "vent") {
      if(state[4] == 0) {
        state[4] = 1;
      } else {
        state[4] = 0;
      }
    } else if (command == "mpv") { // previously purge 
      if(state[5] == 0) {
        state[5] = 1;
      } else {
        state[5] = 0;
      }
    } else if (command == "qd") {
      if(state[6] == 0) {
        state[6] = 1;
      } else {
        state[6] = 0;
      }
    } else if (command == "ignite") { // previous command was MPV
      // NOTE: ASK ABOUT MISSING INDEX HERE
      if(state[8] == 0) {
        state[8] = 1;
      } else {
        state[8] = 0;
      }
    } else if (command == "abort") {
      if(state[9] == 0) {
        state[9] = 1;
      }
    }
    else {
      Serial.println("Unable to recognise invalid command");
    }
  }

void debugPrint(String values, int length1) {
    int length = length1 - 2;
    // print top of box
    for (int i = 0; i < length; i++) {
      Serial.print("+----------");
    }
    Serial.println("+");
    // print headers
    //String headers[] = {"AbortValve", "QD", "Vent", "Ignite", "Purge", "Fill", "Dump", "Heatpad", "MPV", "Siren"};
    String headers[] = {"AbortValve", "QD", "Vent", "MPV", "Fill", "Dump", "Ignite", "Siren"};
    for (int i = 0; i < length; ++i) {
      char buffer[12];
      sprintf(buffer, "%11s", headers[i]);
      Serial.print(buffer);
    }
    Serial.println();
    for (int i = 0; i < length; i++) {
      Serial.print("+----------");
    }
    Serial.println("+");
    // print individual received values
    for (int i = 1; i < length1 + 1; ++i) {
      if(i == 4 || i == 8) {
        continue;
      } else {
        char buffer[12];
//        sprintf(buffer, "%6c%5s", values[i], "");
        sprintf(buffer, "%6c%5s", values[i], "");
        // Print on bttom
        Serial.print(buffer);
      }
    }
    Serial.println();
    // print bottom of box
    for (int i = 0; i < length; i++) {
      Serial.print("+----------");
    }
    Serial.println("+");
  }

// Set all pixels to a certain color
void setAllPixels(uint32_t color) {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, color);
    }
}

void blinkColor(uint32_t color, uint8_t wait, int times) {

    for (int i = 0; i < times; i++) {
        // Turn on all LEDs
        setAllPixels(color);
        strip.show();
        delay(wait);
        // Turn off all LEDs
        setAllPixels(strip.Color(0, 0, 0, 0));
        strip.show();
        delay(wait);
    }
}

void pulseRed(uint8_t wait) {
    for(int j=0; j<256; j++) { // Ramp up from 0 to 255
      strip.fill(strip.Color(j, 0, 0));
      strip.show();
      delay(wait);
    }
    for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
      strip.fill(strip.Color(j, 0, 0));
      strip.show();
      delay(wait);
    }
}

void loop() {
  // connect to server again if the connection was broken
  connect_client();

  // send a message to the controls box
  if((millis() - last_time) > delay_time)
  {
    String message;

    String AbortValve = String(state[9]);
    String Fill = String(state[0]);
    String Purge = String(state[5]); // skip
    String Dump = String(state[3]); //open at 0
    String Vent = String(state[4]); //open at 0
    String QD = String (state[6]);
    String Ignite = String(state[7]); // skip
    String MPV = String(state[8]);
    String Heatpad = String(state[1]); //skip
    String Siren = String(state[2]); // skip

    message = ('A' + AbortValve + QD + Vent + Ignite + Purge + Fill + Dump + Heatpad + MPV + Siren + 'Z');
    
    client.publish(SWITCHBOX_TOPIC, message.c_str());

    Serial.println("Sent:");
    Serial.println(message);
    
    debugPrint(message, STATE_LENGTH);

    last_time = millis();
  }


  // check for updates from user
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');  // Read user input until newline
    input.trim();  // Trim leading/trailing whitespace
    executeCommand(input);
  }

  setAllPixels(strip.Color(0, 0, 0, 0));
  strip.show();

  // Abort valve
  if (state[9] == 1) {
    pulseRed(5);
    prevState[9] = state[9];
  }
  else {
    
    // Fill
    if (state[0] != prevState[0]) {
        if (state[0] == 1) {
        blinkColor(strip.Color(0, 0, 255), 200, 2); // Blue blink for Fill on
        } else {
        blinkColor(strip.Color(255, 255, 0), 200, 1); // Yellow blink for Fill off
        }
        prevState[0] = state[0];
    }

    // Dump valve
    if (state[3] != prevState[3]) {
        if (state[3] == 1) {
        blinkColor(strip.Color(0, 0, 255), 200, 2); 
        } else {
        blinkColor(strip.Color(255, 255, 0), 200, 1);
        }
        prevState[3] = state[3];
    }
    
    // Vent valve
    if (state[4] != prevState[4]) {
        if (state[4] == 1) {
        blinkColor(strip.Color(0, 0, 255), 200, 2); 
        } else {
        blinkColor(strip.Color(255, 255, 0), 200, 1); 
        }
        prevState[4] = state[4];
    }
    
    // QD
    if (state[6] != prevState[6]) {
        if (state[6] == 1) {
        blinkColor(strip.Color(0, 0, 255), 200, 2); 
        } else {
        blinkColor(strip.Color(255, 255, 0), 200, 1);
        }
        prevState[6] = state[6];
    }

    // Ignite
    if (state[7] != prevState[7]) {
        if (state[7] == 1) {
        blinkColor(strip.Color(0, 0, 255), 200, 2);
        } else {
        blinkColor(strip.Color(255, 255, 0), 200, 1); 
        }
        prevState[7] = state[7];
    }

    
    // MPV
    if (state[8] != prevState[8]) {
        if (state[8] == 1) {
        blinkColor(strip.Color(0, 0, 255), 200, 2);
        } else {
        blinkColor(strip.Color(255, 255, 0), 200, 1);
        }
        prevState[8] = state[8];
    }
  }

}