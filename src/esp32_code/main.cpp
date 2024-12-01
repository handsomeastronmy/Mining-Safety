#define BLYNK_TEMPLATE_ID "TMPL2eiVZOmcs"
#define BLYNK_TEMPLATE_NAME "ESPData"
#define BLYNK_AUTH_TOKEN "MupeH3xOrfox_UeyqfiBs9mFYLuHlMkC"

#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include "HX711.h"
#include "soc/rtc.h"
#include <math.h>

// Red LED Pin
#define RED_PIN 12

// Blynk Virtual Pins
#define CO V4
#define X V1
#define Y V2
#define Z V3
#define Vibration V0
#define Pressure V5

// MQ Sensor Configuration
#define MQ_PIN 34
#define RL_VALUE 5
#define RO_CLEAN_AIR_FACTOR 9.83
#define CALIBRATION_SAMPLE_TIMES 50
#define CALIBRATION_SAMPLE_INTERVAL 500
#define READ_SAMPLE_INTERVAL 50
#define READ_SAMPLE_TIMES 5
float Ro = 10; // Initial resistance in clean air

// Gas Curve for CO
float COCurve[3] = {2.3, 0.72, -0.34};

// ADXL345 Accelerometer Object
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

// WiFi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "مباشر من روسيا";
char pass[] = "12312335";

// Google Sheets Script Web App URL
const char* serverName = "https://script.google.com/macros/s/AKfycby_BVlFqT40emTKd0EoX72s44FhXkoaXBRwBD7f93b4Hvb3haPNenVa4jcQqc2rRW4v/exec";

// Load Cell Configuration (Pins and Object)
const int LOADCELL_DOUT_PIN = 18;
const int LOADCELL_SCK_PIN = 5;
HX711 scale;

// Thresholds
const float VIBRATION_THRESHOLD = 5.0; // m/sec
const float CO_THRESHOLD = 25.0; // ppm (estimated value for CO)

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    delay(1000);
    Blynk.begin(auth, ssid, pass);

    // Initialize MQ Sensor
    Serial.println("Calibrating MQ sensor...");
    Ro = MQCalibration(MQ_PIN);
    if (Ro <= 0) {
        Ro = 10; // Fallback value
        Serial.println("Calibration failed. Using default Ro value.");
    }
    Serial.print("MQ Sensor Calibrated. Ro = ");
    Serial.println(Ro);

    // Initialize ADXL345 Accelerometer
    if (!accel.begin()) {
        Serial.println("Failed to find ADXL345 chip");
        while (1); // Stop if accelerometer is not found
    }
    accel.setRange(ADXL345_RANGE_16_G); // Set accelerometer range to ±16g
    Serial.println("ADXL345 connection successful");

    // Initialize Load Cell
    Serial.println("Initializing the scale");
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(17 / 2000); // Set calibration factor
    scale.tare(); // Reset scale to zero

    // Initialize Red LED pin
    pinMode(RED_PIN, OUTPUT);
    digitalWrite(RED_PIN, LOW); // Turn off the Red LED initially
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverName);

        // Get CO value from MQ sensor
        float rs_ro_ratio = MQRead(MQ_PIN) / Ro;
        float coValue = MQGetGasPercentage(rs_ro_ratio, COCurve);

        if (coValue < 0) {
            Serial.println("Invalid CO value. Skipping.");
            coValue = 0; // Fallback value
        }

        Serial.print("CO: ");
        Serial.print(coValue);
        Serial.println(" ppm");
        Blynk.virtualWrite(CO, coValue);

        // Get accelerometer data
        sensors_event_t event;
        accel.getEvent(&event);

        // Send accelerometer data to Blynk
        Blynk.virtualWrite(X, event.acceleration.x);
        Blynk.virtualWrite(Y, event.acceleration.y);
        Blynk.virtualWrite(Z, event.acceleration.z);

        // Calculate vibration intensity (RMS)
        float vibrationIntensity = sqrt((event.acceleration.x * event.acceleration.x +
                                         event.acceleration.y * event.acceleration.y +
                                         event.acceleration.z * event.acceleration.z) / 3);
        Serial.print("Vibration Intensity: ");
        Serial.println(vibrationIntensity);
        Blynk.virtualWrite(Vibration, vibrationIntensity);

        // Turn on Red LED if vibration exceeds threshold
        digitalWrite(RED_PIN, vibrationIntensity > VIBRATION_THRESHOLD);

        // Get weight from load cell
        float weight = scale.get_units(10); // Average of 10 readings
        Serial.print("Weight: ");
        Serial.println(weight, 1);
        Blynk.virtualWrite(Pressure, weight);

        // Prepare JSON payload for server transmission
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["co"] = coValue;
        jsonDoc["x"] = event.acceleration.x;
        jsonDoc["y"] = event.acceleration.y;
        jsonDoc["z"] = event.acceleration.z;
        jsonDoc["vibration"] = vibrationIntensity;
        jsonDoc["weight"] = weight;

        String jsonString;
        serializeJson(jsonDoc, jsonString);

        // Send JSON payload to Google Sheets
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(jsonString);

        // Handle response
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server Response:");
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("WiFi not connected.");
    }

    // Delay to prevent rapid loop execution
    delay(1000);
}

// MQ Sensor Functions
float MQResistanceCalculation(int raw_adc) {
    if (raw_adc == 0) {
        Serial.println("Error: raw_adc is 0.");
        return -1; // Error-indicating value
    }
    return ((float)RL_VALUE * (1023 - raw_adc) / raw_adc);
}

float MQCalibration(int mq_pin) {
    float val = 0;
    for (int i = 0; i < CALIBRATION_SAMPLE_TIMES; i++) {
        float resistance = MQResistanceCalculation(analogRead(mq_pin));
        if (resistance < 0) {
            Serial.println("Invalid resistance during calibration.");
            continue;
        }
        val += resistance;
        delay(CALIBRATION_SAMPLE_INTERVAL);
    }
    val = val / CALIBRATION_SAMPLE_TIMES;
    if (val <= 0) {
        Serial.println("Calibration failed. Resistance is invalid.");
        return -1; // Error-indicating value
    }
    return val / RO_CLEAN_AIR_FACTOR;
}

float MQRead(int mq_pin) {
    float rs = 0;
    for (int i = 0; i < READ_SAMPLE_TIMES; i++) {
        int raw_adc = analogRead(mq_pin);
        if (raw_adc <= 0 || raw_adc >= 1023) {
            Serial.println("Invalid analogRead value: " + String(raw_adc));
            continue;
        }
        rs += MQResistanceCalculation(raw_adc);
        delay(READ_SAMPLE_INTERVAL);
    }
    return rs / READ_SAMPLE_TIMES;
}

float MQGetGasPercentage(float rs_ro_ratio, float *pcurve) {
    if (rs_ro_ratio <= 0) {
        Serial.println("Invalid rs/ro ratio.");
        return -1; // Error-indicating value
    }
    return pow(10, ((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0]);
}
