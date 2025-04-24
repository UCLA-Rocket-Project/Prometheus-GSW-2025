#include <Arduino.h>
#include <ADS1256.h>

#define SPI_MISO 35
#define SPI_SCLK 48
#define SPI_MOSI 34
#define SPI_CS   7
#define DRDY     4
#define LED      38

SPIClass spi_ads(FSPI);
ADS1256 ADC(&spi_ads, DRDY, SPI_CS, 2.5);

// Calibration mode toggle
bool calibrationMode = true;   // Set to true to run calibration

// Sample count and known weights
const int numSamples = 10;
float knownWeights[numSamples] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

// Store measured voltages for both channels
float measuredVoltages0_1[numSamples];
float measuredVoltages2_3[numSamples];

// Calibration coefficients: y = Ax + B for both channels
float calA_0_1 = -195.25664, calB_0_1 = -91.19858;
float calA_2_3 = -195.25664, calB_2_3 = -91.19858;

// Converts voltage to weight based on channel
float convertToWeight(float voltage, int channel) {
  if (channel == 0) return calA_0_1 * voltage + calB_0_1;
  else              return calA_2_3 * voltage + calB_2_3;
}

// Least Squares Linear Regression
void performLinearRegression(float *x, float *y, int n, float &slope, float &intercept) {
  float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
  for (int i = 0; i < n; i++) {
    sumX += x[i];
    sumY += y[i];
    sumXY += x[i] * y[i];
    sumX2 += x[i] * x[i];
  }
  float denom = n * sumX2 - sumX * sumX;
  if (denom == 0) return;

  slope = (n * sumXY - sumX * sumY) / denom;
  intercept = (sumY * sumX2 - sumX * sumXY) / denom;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(SPI_MISO, INPUT_PULLUP);
  pinMode(SPI_SCLK, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(LED, OUTPUT); digitalWrite(LED, HIGH);

  spi_ads.begin(SPI_SCLK, SPI_MISO, SPI_MOSI, SPI_CS);

  ADC.InitializeADC();
  ADC.setPGA(PGA_64);
  ADC.setDRATE(DRATE_1000SPS);

  if (calibrationMode) {
    Serial.println("CALIBRATION MODE");

    for (int i = 0; i < numSamples; i++) {
      Serial.print("Place known weight (lbf): "); Serial.println(knownWeights[i]);
      delay(4000);

      ADC.setMUX(DIFF_0_1);
      measuredVoltages0_1[i] = ADC.convertToVoltage(ADC.readSingle());
      Serial.print("[0-1] Voltage: "); Serial.println(measuredVoltages0_1[i], 6);

      ADC.setMUX(DIFF_2_3);
      measuredVoltages2_3[i] = ADC.convertToVoltage(ADC.readSingle());
      Serial.print("[2-3] Voltage: "); Serial.println(measuredVoltages2_3[i], 6);

      Serial.println();
    }

    performLinearRegression(measuredVoltages0_1, knownWeights, numSamples, calA_0_1, calB_0_1);
    performLinearRegression(measuredVoltages2_3, knownWeights, numSamples, calA_2_3, calB_2_3);

    Serial.println("Calibration Complete:");
    Serial.printf("CH 0-1: A = %.6f, B = %.6f\n", calA_0_1, calB_0_1);
    Serial.printf("CH 2-3: A = %.6f, B = %.6f\n", calA_2_3, calB_2_3);
  }
}

void loop() {
  if (calibrationMode) return;

  ADC.setMUX(DIFF_0_1);
  float v0_1 = ADC.convertToVoltage(ADC.readSingle());
  float w0_1 = convertToWeight(v0_1, 0);

  ADC.setMUX(DIFF_2_3);
  float v2_3 = ADC.convertToVoltage(ADC.readSingle());
  float w2_3 = convertToWeight(v2_3, 1);

  Serial.print("CH[0-1] Voltage: "); Serial.print(v0_1, 6);
  Serial.print(" V\tWeight: "); Serial.print(w0_1, 2); Serial.println(" lbf");

  Serial.print("CH[2-3] Voltage: "); Serial.print(v2_3, 6);
  Serial.print(" V\tWeight: "); Serial.print(w2_3, 2); Serial.println(" lbf");

  delay(1000);
}
