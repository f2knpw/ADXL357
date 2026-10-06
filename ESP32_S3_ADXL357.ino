#include <Arduino.h>
#include <SPI.h>

const int NUM_SENSORS = 3; 

const int PIN_MISO = 13;
const int PIN_MOSI = 11;
const int PIN_SCLK = 12;

const int PIN_SYNC = 14;

const uint8_t CS_PINS[3] = {10, 9, 18};

const uint8_t ADXL357_REG_FIFO_ENTRIES = 0x05;
const uint8_t ADXL357_REG_FIFO_DATA    = 0x11;
const uint8_t ADXL357_REG_FILTER       = 0x28;
const uint8_t ADXL357_REG_SYNC         = 0x2B;
const uint8_t ADXL357_REG_POWER_CTL    = 0x2D;
const uint8_t ADXL357_REG_RESET        = 0x2F;

const uint32_t SPI_FREQ = 4000000;

// Facteur d'échelle pour l'ADXL357 en plage ±10g (51200 LSB/g)
const float LSB_TO_G = 1.0f / 51200.0f; 

struct AccelData {
  float x;
  float y;
  float z;
};

void writeRegister(uint8_t csPin, uint8_t reg, uint8_t value) {
  SPI.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE0));
  digitalWrite(csPin, LOW);
  SPI.transfer((reg << 1) & 0xFE);
  SPI.transfer(value);
  digitalWrite(csPin, HIGH);
  SPI.endTransaction();
}

uint8_t readRegister(uint8_t csPin, uint8_t reg) {
  SPI.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE0));
  digitalWrite(csPin, LOW);
  SPI.transfer((reg << 1) | 0x01);
  uint8_t val = SPI.transfer(0x00);
  digitalWrite(csPin, HIGH);
  SPI.endTransaction();
  return val;
}

AccelData readSensorFIFO(uint8_t csPin) {
  uint8_t rawBuf[9];
  AccelData data = {0.0f, 0.0f, 0.0f};

  SPI.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE0));
  digitalWrite(csPin, LOW);
  
  SPI.transfer((ADXL357_REG_FIFO_DATA << 1) | 0x01);
  for (int i = 0; i < 9; i++) {
    rawBuf[i] = SPI.transfer(0x00);
  }
  
  digitalWrite(csPin, HIGH);
  SPI.endTransaction();

  int32_t rawX = ((uint32_t)rawBuf[0] << 12) | ((uint32_t)rawBuf[1] << 4) | (rawBuf[2] >> 4);
  int32_t rawY = ((uint32_t)rawBuf[3] << 12) | ((uint32_t)rawBuf[4] << 4) | (rawBuf[5] >> 4);
  int32_t rawZ = ((uint32_t)rawBuf[6] << 12) | ((uint32_t)rawBuf[7] << 4) | (rawBuf[8] >> 4);

  if (rawX & 0x80000) rawX |= 0xFFF00000;
  if (rawY & 0x80000) rawY |= 0xFFF00000;
  if (rawZ & 0x80000) rawZ |= 0xFFF00000;

  // Conversion brute -> g
  data.x = (float)rawX * LSB_TO_G;
  data.y = (float)rawY * LSB_TO_G;
  data.z = (float)rawZ * LSB_TO_G;

  return data;
}

void triggerSync() {
  digitalWrite(PIN_SYNC, HIGH);
  delayMicroseconds(5);
  digitalWrite(PIN_SYNC, LOW);
}

void setup() {
  Serial.begin(2000000);
  delay(1000);

  for (int i = 0; i < 3; i++) {
    pinMode(CS_PINS[i], OUTPUT);
    digitalWrite(CS_PINS[i], HIGH);
  }

  pinMode(PIN_SYNC, OUTPUT);
  digitalWrite(PIN_SYNC, LOW);

  SPI.begin(PIN_SCLK, PIN_MISO, PIN_MOSI, -1);
  delay(100);

  for (int i = 0; i < NUM_SENSORS; i++) {
    writeRegister(CS_PINS[i], ADXL357_REG_RESET, 0x52);
    delay(20);
    writeRegister(CS_PINS[i], ADXL357_REG_FILTER, 0x01); // ODR 2000Hz
    writeRegister(CS_PINS[i], ADXL357_REG_SYNC, 0x02);   // Sync externe
    writeRegister(CS_PINS[i], ADXL357_REG_POWER_CTL, 0x00); // Mode mesure
    delay(10);
  }

  Serial.println("Timestamp(us),S1_X(g),S1_Y(g),S1_Z(g),S2_X(g),S2_Y(g),S2_Z(g),S3_X(g),S3_Y(g),S3_Z(g)");
  triggerSync();
}

void loop() {
  static uint32_t lastSyncTime = 0;
  const uint32_t syncIntervalUs = 500; // 2000 Hz

  if ((micros() - lastSyncTime) >= syncIntervalUs) {
    lastSyncTime = micros();

    AccelData samples[3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};

    for (int i = 0; i < NUM_SENSORS; i++) {
      uint8_t entries = readRegister(CS_PINS[i], ADXL357_REG_FIFO_ENTRIES);
      if (entries >= 1) {
        samples[i] = readSensorFIFO(CS_PINS[i]);
      }
    }

    triggerSync();

    uint32_t ts = micros();
    Serial.printf("%lu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                  ts,
                  samples[0].x, samples[0].y, samples[0].z,
                  samples[1].x, samples[1].y, samples[1].z,
                  samples[2].x, samples[2].y, samples[2].z);
  }
}