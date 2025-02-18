#include <Arduino.h>
#include <Profiler.h>


#define NUM_PT 3
#define NUM_LC 2

void setup() {
    Serial.begin(9600);

    float ptVals[NUM_PT] = {0};
    long lcVals[NUM_LC] = {0}; // since read() outputs long (heritage was using floats though)

    String storeStr2 = "Asensorvals";
    {
        profiler_t profiler("Time spent creating storeStr with String");
        
        for (int i = 0; i < NUM_PT; i++) {
            storeStr2 += " pt" + String(i + 1) + "=" + String(ptVals[i]);
        }
        
        for (int i = 3; i < 6; i++) {
            storeStr2 += " pt" + String(i + 1) + "=0";
        }
        
        storeStr2 += " lc1=" + String(lcVals[0]);
        storeStr2 += " lc2=0Z";
    }
    
    {
        profiler_t profiler("Time spent printing to Serial");
        Serial.println(storeStr2);
    }
    
    Serial.println("---------------------------------");

    const int buffer_size = 200 * 30;
    char storeStr[buffer_size] = "Asensorvals pt1=";
    {
      profiler_t profiler("Time spent creating storeStr with c-str");

      char float_buffer[30];
      for (int i = 0; i < NUM_PT; ++i) {
        if (i == NUM_PT - 1) {
          sprintf(float_buffer, "%.2f", ptVals[i]);
        }
        else {
          sprintf(float_buffer, "%.2f,pt%d=", ptVals[i], i + 2);
        }

        strcat(storeStr, float_buffer);
      }
      
      strcat(storeStr, ",pt4=0,pt5=0,pt6=0,lc1=");


      sprintf(float_buffer, "%f,lc2=%fZ", lcVals[0], 0);
      strcat(storeStr, float_buffer);
    }
    {
      profiler_t profiler("Time spent printing to Serial");
      Serial.println(storeStr);
    }
    
    Serial.println("---------------------------------");

    

}   

void loop() {

}