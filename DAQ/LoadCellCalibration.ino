// LOAD CELL CALIBRATION CODE //

#include <HX711.h>

// HX711 LOAD CELL PINS
const int LC_DOUT_PIN = 2;
const int LC_SCK_PIN = 15;

// LOAD CELL
HX711 LC;

// LOAD CELL CALIBRATION FACTORS
float lc_cal_factor = 995; //random number

void setup() {

  Serial.begin(9600);
  Serial.println("HX711 Calibration Sketch");
  Serial.println("remove all weight from load cell");
  Serial.println("after readings begin, place known weight on load cell");
  Serial.println("press + or a to increase calibration factor");
  Serial.println("press - or z to decrease calibration factor");
  
  // Load Cell Measurement
  LC.begin(LC_DOUT_PIN, LC_SCK_PIN);
  LC.set_scale(lc_cal_factor);
  LC.tare(); // reset scale to 0
//  Serial.println("Determining Zero Weight Offset: ");
//  LC.tare(20); // average 20 measurements
//  long offset = LC.get_offset();

//  Serial.print("OFFSET: ");
//  Serial.println(offset);
//  Serial.println();

  long zero_factor = LC.read_average(); // get a baseline reading
  Serial.print("Zero Factor: "); // this can be used to remove the need to tare the scale (useful in permanent scale projects)
  Serial.println(zero_factor);
  
}

void loop() {

  LC.set_scale(lc_cal_factor); // adjust to this calibration factor

  Serial.print("Reading: ");
  Serial.print(LC.get_units());
  Serial.print(" lbs"); // change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.println();
  Serial.print("Calibration Factor: ");
  Serial.print(lc_cal_factor);
  Serial.println();

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      lc_cal_factor += 10;
    else if(temp == '-' || temp == 'z')
      lc_cal_factor -= 10;
  }
}
