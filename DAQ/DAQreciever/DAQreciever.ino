#include <HardwareSerial.h>
#include <string>
#define RO_PIN 16
#define DI_PIN 17
unsigned long long delay_time = 250;
unsigned long long last_time = 0;
HardwareSerial rs485Serial(2);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  digitalWrite(22, LOW);//on
  digitalWrite(23, LOW);//off
}
void loop() {
  // put your main code here, to run repeatedly:
  while (!rs485Serial.available() && millis() - last_time > 1000){
    Serial.println("Unavailable...");
  }
  while (rs485Serial.available()){
    char received = rs485Serial.read();
      //label:
      String message;
      if (received == 'A'){
        for (int i = 0; i < 300; i++){
          received = rs485Serial.read();
          if (received == 'Z'){
            break;
          }
          message += String(received);
        }
        //Serial.println("Recieved:");
        String printMsg = "A" + String(message) + "Z";
        Serial.println(printMsg);
    }
    last_time = millis();
  }
}