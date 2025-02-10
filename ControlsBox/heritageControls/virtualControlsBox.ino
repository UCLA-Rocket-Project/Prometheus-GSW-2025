// THIS IS MEANT FOR THE CONTROLS BOX, IN THE EVENT THAT THE ETHERNET CONNECTION DOESN'T WORK

#include <Arduino.h>
#include <assert.h>

#define outlet 2
#define fill 22
#define dump 21 
#define vent 19
#define qd 18
#define purge 5
#define mpv 4
#define ignite 15
#define abortSiren 23
#define abortValve 13
#define InterruptPin 12

const int stateLength = 9;
int state[stateLength] = {0,0,0,0,0,0,0,0,0};


// this is the code for triggering the desired interrupt
void IRAM_ATTR isr() {
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

  Serial.println("Interrupt Request Received!");
  assert(0);
}

void debugPrint(int length = stateLength) {
  // print top of box
  for (int i = 0; i < length; i++) {
    Serial.print("+----------");
  }
  Serial.println("+");

  // print headers
  String headers[] = {"fill", "outlet", "abortsiren", "dump", "vent", "purge", "qd", "ignite", "mpv" };
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
  for (int i = 0; i < length; ++i) {
    char buffer[12];
    sprintf(buffer, "%6d%5s", state[i], "");
    Serial.print(buffer);
  }
  Serial.println();

  // print bottom of box
  for (int i = 0; i < length; i++) {
    Serial.print("+----------");
  }
  Serial.println("+");
}

void setup() {
 Serial.begin(9600);
 pinMode(InterruptPin, INPUT_PULLUP);
 attachInterrupt(InterruptPin, isr, FALLING);
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
}

void executeCommand(String command) {
  // Convert to lowercase for case-insensitive comparison
  command.toLowerCase();
    if (command == "onfill"){
        digitalWrite(fill, LOW);
        state[0] = 1;
    }
    else if (command == "abort"){
      isr();
    }
    else if (command == "offfill"){
        digitalWrite(fill, HIGH);
        state[0] = 0;
    }
    else if (command=="onoutlet"){
        digitalWrite(outlet, LOW);
        state[1] = 1;
    }
    else if (command=="offoutlet"){
        digitalWrite(outlet, HIGH);
        state[1] = 0;
    }
    else if (command == "onabortsiren"){
        digitalWrite(abortSiren, HIGH);
        state[2] = 1;
    }
    else if (command=="offabortsiren"){
        digitalWrite(abortSiren,LOW);
        state[2] = 0;
    }
    else if (command=="ondump"){
        digitalWrite(dump, LOW);
        state[3] = 1;
    }
    else if (command=="offdump"){
        digitalWrite(dump, HIGH);
        state[3] = 0;
    }
    else if (command=="onvent"){
        digitalWrite(vent, LOW);
        state[4] = 1;
    }
    else if (command=="offvent"){
        digitalWrite(vent, HIGH);
        state[4] = 0;
    }
    else if (command=="onpurge"){
        digitalWrite(purge, LOW);
        state[5] = 1;
    }
    else if (command=="offpurge"){
        digitalWrite(purge,HIGH);
        state[5] = 0;
    }
    else if (command=="onqd"){
        digitalWrite(qd, LOW);
        state[6] = 1;
    }
    else if (command=="offqd"){
        digitalWrite(qd, HIGH);
        state[6] = 0;
    }
    else if (command=="onignite"){
        digitalWrite(ignite, HIGH);
        state[7] = 1;
    }
    else if (command=="offignite"){
        digitalWrite(ignite, LOW);
        state[7] = 0;
    }
    else if (command=="onmpv"){
        digitalWrite(mpv, LOW);
        state[8] = 1;
    }
    else if (command=="offmpv"){
        digitalWrite(mpv, HIGH);
        state[8] = 0;
    }
    else{
    Serial.println("Unable to recognise invalid command");
  }
}

void loop() {
 delay(1000);
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');  // Read user input until newline
    input.trim();  // Trim leading/trailing whitespace
    executeCommand(input);
    }
  debugPrint();
} 
