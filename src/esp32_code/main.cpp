#define BLYNK_TEMPLATE_ID "TMPL2eiVZOmcs"
#define BLYNK_TEMPLATE_NAME "ESPData"
#define BLYNK_AUTH_TOKEN "MupeH3xOrfox_UeyqfiBs9mFYLuHlMkC"

#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_ADXL345_U.h>
#include <math.h>
#include <DHT.h>
#include <DHT_U.h>

// Red LED Pin
#define RED_PIN 12

// Blynk Virtual Pins
#define CO V4
#define X V1
#define Y V2
#define Z V3
#define Vibration V0
#define Humidity V6
#define Tiltrollangle V7

// MQ Sensor Configuration
#define MQ_PIN 34
#define RL_VALUE 5
#define RO_CLEAN_AIR_FACTOR 9.83
#define CALIBRATION_SAMPLE_TIMES 50
#define CALIBRATION_SAMPLE_INTERVAL 500
#define READ_SAMPLE_INTERVAL 50
#define READ_SAMPLE_TIMES 5
float Ro = 10; // Initial resistance in clean air
float COCurve[3] = {2.3, 0.72, -0.34}; // Gas Curve for CO

// ADXL345 Accelerometer Object
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
float initialX = 0.0, initialY = 0.0, initialZ = 0.0;

// WiFi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "11310";
char pass[] = "12312335";

// Google Sheets Script Web App URL
const char* serverName = "https://script.google.com/macros/s/AKfycbxQk6eSnsTtMaqnAH6qPdIR_RvmXM1aUqoxLsdW7h6LN62MFMIkM_Q8Y4trZ-idLMjaug/exec"; // Replace with your Apps Script Web App URL

DHT dht1(4, DHT11);

// Thresholds
const float VIBRATION_THRESHOLD = 6; // m/sec'2
const float CO_THRESHOLD = 25.0; // ppm (estimated value for CO)
const float Humidity_THRESHOLD = 84;
const float Roll_THRESHOLD = 23;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(100);

  Blynk.begin(auth, ssid, pass);

  if (!accel.begin()) {
    Serial.println("Failed to find ADXL345 chip");
    while (1); // Stop if accelerometer is not found
  }
  accel.setRange(ADXL345_RANGE_2_G); // Set accelerometer range to ±16g

  // Capture initial (baseline) accelerometer values
  sensors_event_t event;
  accel.getEvent(&event);
  initialX = event.acceleration.x;
  initialY = event.acceleration.y;
  initialZ = event.acceleration.z;
  Serial.println("Initial accelerometer readings captured");

  pinMode(RED_PIN, OUTPUT);

  // Initialize MQ Sensor
  Serial.println("Calibrating MQ sensor...");
  Ro = MQCalibration(MQ_PIN);
  if (Ro <= 0) {
    Ro = 10; // Fallback value
    Serial.println("Calibration failed. Using default Ro value.");
  }
  Serial.print("MQ Sensor Calibrated. Ro = ");
  Serial.println(Ro);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float rs_ro_ratio = MQRead(MQ_PIN) / Ro;
    float coValue = MQGetGasPercentage(rs_ro_ratio, COCurve);
    if (coValue < 0) coValue = 0; // Fallback value

    float humidity = dht1.readHumidity();
    sensors_event_t event;
    accel.getEvent(&event);

    // Adjusted accelerometer values
    float adjustedX = event.acceleration.x - initialX;
    float adjustedY = event.acceleration.y - initialY;
    float adjustedZ = event.acceleration.z - initialZ;

    // Calculate vibration intensity (RMS)
    float vibrationIntensity = sqrt((adjustedX * adjustedX + adjustedY * adjustedY + adjustedZ * adjustedZ) / 3);

    // Calculate tilt angles
    float roll = atan2(event.acceleration.y, event.acceleration.z) * 180.0 / PI;
    // Write to Blynk
    Blynk.virtualWrite(CO, coValue);
    Blynk.virtualWrite(Vibration, vibrationIntensity);
    Blynk.virtualWrite(Humidity, humidity);
    Blynk.virtualWrite(Tiltrollangle, roll);
    Blynk.virtualWrite(Tiltpitchangle, pitch);

    // LED Alert
    if (vibrationIntensity > VIBRATION_THRESHOLD) digitalWrite(RED_PIN, HIGH);
    else digitalWrite(RED_PIN, LOW);

    // Check thresholds and print alert messages
    if (coValue > CO_THRESHOLD) {
      Serial.println("Alert: CO levels are above the threshold!");
    }

    if (vibrationIntensity > VIBRATION_THRESHOLD) {
      Serial.println("Alert: Vibration levels are above the threshold!");
    }

    if (humidity > Humidity_THRESHOLD) {
      Serial.println("Alert: Humidity levels are above the threshold!");
    }

    if (roll > Roll_THRESHOLD) {
      Serial.println("Alert: Roll angle is above the threshold!");
    }

    // Send Data to Google Sheets
    sendDataToGoogleSheets(coValue, vibrationIntensity, humidity, roll, pitch);
  } else {
    Serial.println("WiFi not connected.");
  }

  delay(1000); // Delay to prevent rapid loop execution
}

// Function to send data to Google Sheets
void sendDataToGoogleSheets(float coValue, float vibrationIntensity, float humidity, float roll, float pitch) {
  HTTPClient http;
  http.begin(serverName);

  // Prepare JSON payload
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["co"] = coValue;
  jsonDoc["vibration Intensity"] = vibrationIntensity;
  jsonDoc["Humidity"] = humidity;
  jsonDoc["Roll"] = roll;
  Serial.print(humidity);
  Serial.print(roll);
  Serial.print(coValue);
  Serial.print(vibrationIntensity);

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Send HTTP POST request
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    Serial.println("Google Sheets Response: " + http.getString());
  } else {
    Serial.print("Error sending data: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// MQ Sensor Functions
float MQResistanceCalculation(int raw_adc) {
  if (raw_adc == 0) return -1; // Error-indicating value
  return ((float)RL_VALUE * (1023 - raw_adc) / raw_adc);
}

float MQCalibration(int mq_pin) {
  float val = 0;
  for (int i = 0; i < CALIBRATION_SAMPLE_TIMES; i++) {
    float resistance = MQResistanceCalculation(analogRead(mq_pin));
    if (resistance > 0) val += resistance;
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  return val / CALIBRATION_SAMPLE_TIMES / RO_CLEAN_AIR_FACTOR;
}

float MQRead(int mq_pin) {
  float rs = 0;
  for (int i = 0; i < READ_SAMPLE_TIMES; i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
  return rs / READ_SAMPLE_TIMES;
}

float MQGetGasPercentage(float rs_ro_ratio, float *pcurve) {
  return pow(10, ((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0]);
}
