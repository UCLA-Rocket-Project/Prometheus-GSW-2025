// THIS IS MEANT FOR THE SWITCHBOX, IN CASE SOME OF THE SWTICHES DONT WORK AND YOU STILL NEED TO USE THE RS485 CONNECTION

//Changed commands to toggle
#include <HardwareSerial.h> 
#include <Arduino.h>
#include <string>

#define RO_PIN 16 //32 boRD
#define DI_PIN 17 //35
//jumper wire to HIGH

#define RE 26 //13
#define DE 26
//RE to 26
//DE to 27

#define purge 22 //22
#define fill 23 //23
#define abortValve 21 //21
#define dump 19 //19
#define vent 18 //18
#define qd 4 //4 // always on
#define ignite 2 //2
#define mpv 15 //15
#define heatpad 5 //17
#define siren 13 //22
#define sirenPower 32 //23

unsigned long long delay_time = 250;
unsigned long long last_time = 0;


HardwareSerial rs485Serial(2);

const int STATE_LENGTH = 10;
int state[STATE_LENGTH] = {0,0,0,0,0,0,0,0,0,0};

void setup() {
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(RE, HIGH);
  digitalWrite(DE, HIGH); 

  pinMode(purge, INPUT_PULLDOWN);
  pinMode(fill, INPUT_PULLDOWN);
  pinMode(abortValve, INPUT_PULLDOWN);
  pinMode(dump, INPUT_PULLDOWN);
  pinMode(vent, INPUT_PULLDOWN);
  pinMode(qd, INPUT_PULLDOWN);
  pinMode(ignite, INPUT_PULLDOWN);
  pinMode(mpv, INPUT_PULLDOWN);
  pinMode(siren, INPUT_PULLDOWN);
  pinMode(sirenPower, OUTPUT);

  digitalWrite(sirenPower, HIGH);
  
  Serial.begin(115200);
  rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);

  Serial.println("Setup Complete");

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
    } /*else if (command == "heatpad") {
      if(state[1] == 0) {
        state[1] = 1;
      } else {
        state[1] = 0;
      }
    } else if (command == "outlet") {
      if(state[1] == 0) {
        state[1] = 1;
      } else {
        state[1] = 0;
      }
    }*/ else if (command == "siren") {
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
    // } else if (command == "ignite") {
    //   if(state[7] == 0) {
    //     state[7] = 1;
    //   } else {
    //     state[7] = 0;
    //   }
    } else if (command == "ignite") { // previous command was MPV
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
    // if (command == "onfill") {
    //     state[0] = 1;
    // }
    // else if (command == "offfill") {
    //     state[0] = 0;
    // }
    // else if (command == "onheatpad") {
    //     state[1] = 1;
    // }
    // else if (command == "offheatpad") {
    //     state[1] = 0;
    // }
    // else if (command=="onoutlet"){
    //     state[1] = 1;
    // }
    // else if (command=="offoutlet"){
    //     state[1] = 0;
    // }
    // else if (command == "onsiren"){  
    //     state[2] = 1;
    // }
    // else if (command=="offsiren"){
    //     state[2] = 0;
    // }
    // else if (command=="ondump"){
    //     state[3] = 1;
    // }
    // else if (command=="offdump"){
    //     state[3] = 0;
    // }
    // else if (command=="onvent"){
    //     state[4] = 1;
    // }
    // else if (command=="offvent"){
    //     state[4] = 0;
    // }
    // else if (command=="onpurge"){
    //     state[5] = 1;
    // }
    // else if (command=="offpurge"){
    //     state[5] = 0;
    // }
    // else if (command=="onqd"){
    //     state[6] = 1;
    // }
    // else if (command=="offqd"){
    //     state[6] = 0;
    // }
    // else if (command=="onignite"){
    //     state[7] = 1;
    // }
    // else if (command=="offignite"){
    //     state[7] = 0;
    // }
    // else if (command=="onmpv"){
    //     state[8] = 1;
    // }
    // else if (command=="offmpv"){
    //     state[8] = 0;
    // }
    // else if (command == "onabort") {
    //     state[9] = 1;
    // }
    // else if (command == "offabort") {
    //     state[9] = 0;
    // }
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

void loop() {
    // send a message to the controls box
  if((millis() - last_time) > delay_time)
  {
    String message;

    String AbortValve = String(state[9]);
    String Fill = String(state[0]);
    String Purge = String(state[5]);
    String Dump = String(state[3]); //open at 0
    String Vent = String(state[4]); //open at 0
    String QD = String (state[6]);
    String Ignite = String(state[7]); // skip
    String MPV = String(state[8]);
    String Heatpad = String(state[1]); //skip
    String Siren = String(state[2]);

    message = ('A' + AbortValve + QD + Vent + Ignite + Purge + Fill + Dump + Heatpad + MPV + Siren + 'Z');
    
    rs485Serial.println(message);
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
}