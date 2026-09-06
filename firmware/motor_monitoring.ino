#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <math.h>

#include "secrets.h"

// -----------------------------------------------------------------------------
// Hardware configuration
// -----------------------------------------------------------------------------

#define RELAY_PIN       21
#define ACS712_PIN      4
#define LM35_PIN        1
#define HALL_DO_PIN     10
#define LED_GREEN       2
#define LED_RED         3

#define I2C_SDA_PIN     8
#define I2C_SCL_PIN     9

#define MPU6050_ADDRESS 0x68
#define LCD_ADDRESS     0x27

// -----------------------------------------------------------------------------
// Monitoring thresholds
// -----------------------------------------------------------------------------

const float TEMPERATURE_LIMIT_C = 40.0;
const float CURRENT_LIMIT_A = 10.0;
const float VIBRATION_FACTOR = 1.20;

const float ACS712_SENSITIVITY_V_PER_A = 0.066;
const float CURRENT_ZERO_THRESHOLD_A = 0.5;

#define FILTER_SAMPLES 20

// Timing
const unsigned long RPM_INTERVAL_MS = 1000;
const unsigned long GOOGLE_LOG_INTERVAL_MS = 20000;
const unsigned long LCD_INTERVAL_MS = 400;
const unsigned long BLYNK_INTERVAL_MS = 1000;

// -----------------------------------------------------------------------------
// Objects
// -----------------------------------------------------------------------------

MPU6050 mpu(MPU6050_ADDRESS);
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

// -----------------------------------------------------------------------------
// Runtime variables
// -----------------------------------------------------------------------------

bool mpuConnected = false;

float ACS712_offset = 0.0;
float currentBuffer[FILTER_SAMPLES];
int bufferIndex = 0;

float vibrationBase = 0.0;

volatile unsigned long pulseCount = 0;
int motorRPM = 0;

unsigned long lastRPMTime = 0;
unsigned long lastGoogleLogTime = 0;
unsigned long lastLCDTime = 0;
unsigned long lastBlynkTime = 0;

// -----------------------------------------------------------------------------
// Hall sensor interrupt
// -----------------------------------------------------------------------------

void IRAM_ATTR countPulse() {
  pulseCount++;
}

// -----------------------------------------------------------------------------
// ACS712 calibration
// -----------------------------------------------------------------------------

void calibrateACS712() {
  long sum = 0;
  const int samples = 500;

  for (int i = 0; i < samples; i++) {
    int raw = analogRead(ACS712_PIN);
    float voltage = raw * (3.3 / 4095.0);

    sum += (long)(voltage * 1000.0);
    delay(2);
  }

  ACS712_offset = (sum / (float)samples) / 1000.0;
}

// -----------------------------------------------------------------------------
// MPU6050 vibration calibration
// -----------------------------------------------------------------------------

void calibrateVibration() {
  long double sum = 0;
  const int samples = 500;

  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az, gx, gy, gz;

    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    float magnitude = sqrt(
      (float)ax * ax +
      (float)ay * ay +
      (float)az * az
    );

    sum += magnitude;
    delay(2);
  }

  vibrationBase = (float)(sum / samples);
}

// -----------------------------------------------------------------------------
// Current measurement
// -----------------------------------------------------------------------------

float readCurrent() {
  int raw = analogRead(ACS712_PIN);
  float voltageACS = raw * (3.3 / 4095.0);

  float currentInstant =
    (voltageACS - ACS712_offset) /
    ACS712_SENSITIVITY_V_PER_A;

  currentBuffer[bufferIndex] = currentInstant;
  bufferIndex = (bufferIndex + 1) % FILTER_SAMPLES;

  float current = 0.0;

  for (int i = 0; i < FILTER_SAMPLES; i++) {
    current += currentBuffer[i];
  }

  current /= FILTER_SAMPLES;

  if (fabs(current) < CURRENT_ZERO_THRESHOLD_A) {
    current = 0.0;
  }

  return current;
}

// -----------------------------------------------------------------------------
// Temperature measurement
// -----------------------------------------------------------------------------

float readTemperature() {
  int rawLM35 = analogRead(LM35_PIN);
  float voltageLM35 = rawLM35 * (3.3 / 4095.0);

  // LM35: approximately 10 mV / °C
  return voltageLM35 * 100.0;
}

// -----------------------------------------------------------------------------
// Vibration measurement
// -----------------------------------------------------------------------------

float readVibration() {
  if (!mpuConnected) {
    return 0.0;
  }

  int16_t ax, ay, az, gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  return sqrt(
    (float)ax * ax +
    (float)ay * ay +
    (float)az * az
  );
}

// -----------------------------------------------------------------------------
// RPM calculation
// -----------------------------------------------------------------------------

void updateRPM() {
  unsigned long now = millis();

  if (now - lastRPMTime >= RPM_INTERVAL_MS) {
    noInterrupts();
    unsigned long pulses = pulseCount;
    pulseCount = 0;
    interrupts();

    // Assumes one Hall pulse per motor revolution.
    motorRPM = (int)(pulses * 60UL);

    lastRPMTime = now;
  }
}

// -----------------------------------------------------------------------------
// Fault detection
// -----------------------------------------------------------------------------

bool checkFault(
  float current,
  float temperature,
  float vibration
) {
  bool overTemperature = temperature > TEMPERATURE_LIMIT_C;
  bool overCurrent = current > CURRENT_LIMIT_A;
  bool excessiveVibration =
    mpuConnected &&
    vibration > vibrationBase * VIBRATION_FACTOR;

  return overTemperature || overCurrent || excessiveVibration;
}

// -----------------------------------------------------------------------------
// Motor protection and status LEDs
// -----------------------------------------------------------------------------

void updateProtection(bool fault) {
  if (fault) {
    digitalWrite(RELAY_PIN, HIGH);   // Motor OFF
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);    // Motor ON
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
  }
}

// -----------------------------------------------------------------------------
// LCD
// -----------------------------------------------------------------------------

void updateLCD(float current, float temperature, float vibration, bool fault) {
  lcd.setCursor(0, 0);
  lcd.print("I:");
  lcd.print(current, 1);
  lcd.print("A T:");
  lcd.print(temperature, 0);
  lcd.print("C ");

  lcd.setCursor(0, 1);
  lcd.print("RPM:");
  lcd.print(motorRPM);
  lcd.print(" V:");
  lcd.print((int)vibration);
  lcd.print(fault ? " F" : " N");
}

// -----------------------------------------------------------------------------
// Serial output
// -----------------------------------------------------------------------------

void printSerialData(
  float current,
  float temperature,
  float vibration
) {
  Serial.print("Current: ");
  Serial.print(current, 2);
  Serial.print(" A | ");

  Serial.print("Temp: ");
  Serial.print(temperature, 1);
  Serial.print(" C | ");

  Serial.print("RPM: ");
  Serial.print(motorRPM);
  Serial.print(" | ");

  Serial.print("Vibration: ");
  Serial.println(vibration, 0);

  // Serial Plotter format
  Serial.print(current);
  Serial.print('\t');
  Serial.print(temperature);
  Serial.print('\t');
  Serial.print(motorRPM);
  Serial.print('\t');
  Serial.println(vibration);
}

// -----------------------------------------------------------------------------
// Blynk update
// -----------------------------------------------------------------------------

void updateBlynk(
  float current,
  float temperature,
  float vibration,
  bool fault
) {
  Blynk.virtualWrite(V3, current);
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V4, motorRPM);
  Blynk.virtualWrite(V1, vibration);
  Blynk.virtualWrite(V5, fault ? 1 : 0);
}

// -----------------------------------------------------------------------------
// Google Sheets logging
// -----------------------------------------------------------------------------

void sendToGoogleSheet(
  float current,
  float temperature,
  int rpm,
  float vibration,
  bool fault
) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  HTTPClient http;

  http.begin(GOOGLE_SCRIPT_URL);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{";
  jsonData += "\"current\":" + String(current, 3) + ",";
  jsonData += "\"temp\":" + String(temperature, 2) + ",";
  jsonData += "\"rpm\":" + String(rpm) + ",";
  jsonData += "\"vibration\":" + String(vibration, 2) + ",";
  jsonData += "\"status\":\"";
  jsonData += fault ? "FAULT" : "NORMAL";
  jsonData += "\"";
  jsonData += "}";

  int httpResponseCode = http.POST(jsonData);

  Serial.print("Google Sheets HTTP response: ");
  Serial.println(httpResponseCode);

  http.end();
}

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  pinMode(HALL_DO_PIN, INPUT);

  attachInterrupt(
    digitalPinToInterrupt(HALL_DO_PIN),
    countPulse,
    RISING
  );

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // MPU6050
  mpu.initialize();

  uint8_t whoami = mpu.getDeviceID();

  mpuConnected =
    (whoami == 0x68 ||
     whoami == 0x70 ||
     whoami == 0x38);

  // LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
  delay(2000);

  lcd.clear();

  // Sensor calibration
  calibrateACS712();

  if (mpuConnected) {
    calibrateVibration();
  }

  // Initialize current filter
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    currentBuffer[i] = 0.0;
  }

  analogSetPinAttenuation(LM35_PIN, ADC_11db);

  // Initial safe/normal state
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Motor Health OK");

  Serial.println();
  Serial.println("Motor Monitoring System");
  Serial.println("-----------------------");
  Serial.print("MPU6050: ");
  Serial.println(mpuConnected ? "Connected" : "Not detected");
  Serial.print("ACS712 offset: ");
  Serial.println(ACS712_offset, 3);
  Serial.print("Vibration baseline: ");
  Serial.println(vibrationBase, 2);

  // Start Blynk / Wi-Fi
  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    WIFI_SSID,
    WIFI_PASSWORD
  );
}

// -----------------------------------------------------------------------------
// Main loop
// -----------------------------------------------------------------------------

void loop() {
  // Keep Blynk connection serviced continuously.
  Blynk.run();

  // Read sensors
  float current = readCurrent();
  float temperature = readTemperature();
  float vibration = readVibration();

  // Update RPM
  updateRPM();

  // Fault detection
  bool fault = checkFault(
    current,
    temperature,
    vibration
  );

  // Protection
  updateProtection(fault);

  // Serial output
  printSerialData(
    current,
    temperature,
    vibration
  );

  unsigned long now = millis();

  // LCD update
  if (now - lastLCDTime >= LCD_INTERVAL_MS) {
    updateLCD(
      current,
      temperature,
      vibration,
      fault
    );

    lastLCDTime = now;
  }

  // Blynk update
  if (now - lastBlynkTime >= BLYNK_INTERVAL_MS) {
    updateBlynk(
      current,
      temperature,
      vibration,
      fault
    );

    lastBlynkTime = now;
  }

  // Google Sheets update
  if (now - lastGoogleLogTime >= GOOGLE_LOG_INTERVAL_MS) {
    sendToGoogleSheet(
      current,
      temperature,
      motorRPM,
      vibration,
      fault
    );

    lastGoogleLogTime = now;
  }
}
