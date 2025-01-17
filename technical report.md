
## 1. Introduction

This project is concerned with mine condition modelling for ensuring safety and stability of mines. The following sections detail the process of developing this system, with the necessary code blocks explained and the power consumption calculations written in Latex for ease of readability.

## 2. Sensor integration
The sensors connections were planned in the Cirkit Designer Software and can be seen in the following figure:
**![](https://lh7-rt.googleusercontent.com/slidesz/AGV_vUfpIIhmLG60DCN_r03ort3DBUqlsi1_eMqJrW-7dWEWY1xa75wYNkm4i9utNh_CxpffEariiI11QVOV7fsfx2c2GiPXY2wcjF_oi_b1JPsGrKyqwjHZiDq-nO8ffOKdcDKABTC2N8qfCDD-LCQV4CE=s2048?key=XSWHYWdthl-CbACJd_wcfhjO)**

The entire code for the system can be shown in:
```c++
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
const float CO_THRESHOLD = 1; // ppm (estimated value for CO)
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
  accel.setRange(ADXL345_RANGE_2_G);

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
    float vibrationIntensity = sqrt((adjustedX * adjustedX + adjustedY * adjustedY + adjustedZ * adjustedZ - 1) / 3);

    // Calculate tilt angles
    float roll = atan2(event.acceleration.y, event.acceleration.z) * 180.0 / PI;
    // Write to Blynk
    Blynk.virtualWrite(CO, coValue);
    Blynk.virtualWrite(Vibration, vibrationIntensity);
    Blynk.virtualWrite(Humidity, humidity);
    Blynk.virtualWrite(Tiltrollangle, roll);

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
    sendDataToGoogleSheets(coValue, vibrationIntensity, humidity, roll);
  } else {
    Serial.println("WiFi not connected.");
  }

  delay(1000); // Delay to prevent rapid loop execution
}

// Function to send data to Google Sheets
void sendDataToGoogleSheets(float coValue, float vibrationIntensity, float humidity, float roll) {
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

```

## 3. Signal interpretation techniques:
The following section will detail the methodology of converting raw sensor data to readings accurately expressing the environmental conditions sent.

### MQ-9 sensor calibration 

The calibration of the MQ-9 sensor consisted of converting the Rs/R0 ratio to Carbon monoxide concentration 

First, R0 is the sensor’s resistance in clean air or baseline. Rs is the sensor’s resistance when exposed to the target gas, which is variable and changes in relation to the amount of gas present.


The calibration involves determining the sensor’s baseline resistance (R0) in clean air. This is essential for calculating the ratio (Rs/R0) used in gas concentration equations.

   ```cpp
   float MQCalibration(int mq_pin) {
       float val = 0;
       for (int i = 0; i < CALIBRATION_SAMPLE_TIMES; i++) {
           float resistance = MQResistanceCalculation(analogRead(mq_pin));
           val += resistance;
           delay(CALIBRATION_SAMPLE_INTERVAL);
       }
       val = val / CALIBRATION_SAMPLE_TIMES;
       return val / RO_CLEAN_AIR_FACTOR;
   }
   ```

The following function computes the sensor’s resistance based on the analog reading from the sensor.

```cpp
float MQResistanceCalculation(int raw_adc) {
    return ((float)RL_VALUE * (1023 - raw_adc) / raw_adc);
}
```

The gas curve is an array that represents the sensor's response to a specific gas, where the Rs/R0 values are plotted in certain conditions following a logarithmic patterns with the curve for each gas available in the datasheets. 

![[Pasted image 20241205101302.png]]

In case of CO the array is:

   ```cpp
   float COCurve[3] = {2.3, 0.72, -0.34};
   ```

Where:
- COCurve[0] is the y-intercept of the curve.
- COCurve[1] is the slope or the gradient of the curve.
- COCurve[2] The exponent in the logarithmic equation.

These values are used to convert the Rs/R0 ratio into a gas concentration (in ppm) using the following logarithmic formula:
   ```cpp
   float MQGetGasPercentage(float rs_ro_ratio, float *pcurve) {
       return pow(10, ((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0]);
   }
   ```

The equation can be represented as follows:

$$
C = 10^{\left( \frac{\log\left( \frac{R_s}{R_0} \right) - \log(B)}{m} \right)}
$$


$$
C = 10^{\left( \frac{\log\left( \frac{R_s}{R_0} \right) - 0.72}{-0.34} + 2.3 \right )}
$$



Where:
- \( C \) represents the gas concentration.
- \( Rs/R0 \) is the ratio of the sensor resistance to the clean air resistance.
- \( B \) is the y-intercept of the plot (the base value).
- \( m \) is the slope of the plot. 

The equation is represented as such as the Rs/R0 ratio varies logarithmically with the target gas concentration.

### ADXL345 Accelerometer Calibration 

This codeblock initializes ADXL345 sensor and determines it's sensitivity range, chosen as (±2g), for precise vibration measurement.

   ```cpp
   // Initialize the ADXL345 accelerometer
   if (!accel.begin()) {
       Serial.println("Failed to find ADXL345 chip");
       while (1); // Stop if accelerometer is not found
   }
   accel.setRange(ADXL345_RANGE_2_G); // Set accelerometer range to ±2g
   ```

The baseline values of the static initial accelerometer readings in order to account for the static offsets that might affect measurements.

   ```cpp
   sensors_event_t event;
   accel.getEvent(&event);
   initialX = event.acceleration.x;
   initialY = event.acceleration.y;
   initialZ = event.acceleration.z;
   Serial.println("Initial accelerometer readings captured");
   ```

The adjust acceleration values were obtained from subtracting the current readings from the baseline values so that the adjusted readings represent the true change in acceleration or movement:

   ```cpp
   sensors_event_t event;
   accel.getEvent(&event);

   // Adjusted accelerometer values
   float adjustedX = event.acceleration.x - initialX;
   float adjustedY = event.acceleration.y - initialY;
   float adjustedZ = event.acceleration.z - initialZ;
   ```

The vibration intensity was calculated by obtaining the root mean square of the acceleration along the x,y and z axes to represent the magnitude of the vibrations. The subtraction of 1 accounts for the gravity component measured as 1 g-force unit in the z-axis which is always present even when the accelerometer is static.

   ```cpp
   float vibrationIntensity = sqrt((adjustedX * adjustedX + adjustedY * adjustedY + adjustedZ * adjustedZ - 1) / 3);
   ```

The roll angle is the angle of rotation about the reference axis, which is considered an important parameter contributing to slope stability in mines. The following code snippet shows how it has been computed using the inverse tangent trigonometric function:

```cpp
// Capture initial (baseline) accelerometer values
sensors_event_t event;
accel.getEvent(&event);
initialX = event.acceleration.x;
initialY = event.acceleration.y;
initialZ = event.acceleration.z;
Serial.println("Initial accelerometer readings captured");

// In the loop function
sensors_event_t event;
accel.getEvent(&event);

// Adjusted accelerometer values
float adjustedX = event.acceleration.x - initialX;
float adjustedY = event.acceleration.y - initialY;
float adjustedZ = event.acceleration.z - initialZ;

// Calculate tilt angles
float roll = atan2(event.acceleration.y, event.acceleration.z) * 180.0 / PI;

// Write to Blynk
Blynk.virtualWrite(Tiltrollangle, roll);

```


## 4. Power Efficiency

A power supplying system is to be designed so that the sensors can operate for 6 continuous months without battery replacement. The following section will discuss the power consumption calculation to choose a battery with a suitable rating.

It is set that the duty cycle (how often the sensor is active), be 10% for energy saving, which will result in 6 readings per minute for the 3 sensors. seconds)

Now since we are using the ESP32 power pin, the operating voltage for each of the 3 sensors will be 3.3 V.

 Given the DHT11 sensor consumes 2.5mA (0.0025A) during active transmission, its average power consumption will be the product of the operating voltage, current and the 10% duty cycle:

$$
P_{\text{avg, DHT11}} = 3.3\,\text{V} \times 0.0025\,\text{A} \times 0.1 = 0.000825\,\text{W}
$$

Similarly for the accelerometer with 1.5mA (0.0015A) current consumption:

$$
P_{\text{avg, accelerometer}} = 3.3\,\text{V} \times 0.0015\,\text{A} \times 0.1 = 0.000495\,\text{W}
$$

And the MQ-9 sensor with 30mA (0.03A) current consumption:

$$
P_{\text{avg, MQ9}} = 3.3\,\text{V} \times 0.03\,\text{A} \times 0.1 = 0.0099\,\text{W}
$$

Now we will calculate the energy consumption per year by multiplying the average power by 8760 hours (hours in 1 year) for the resulting energy consumptions as follows:

$$
E_{\text{DHT11}} = 0.000825\,\text{W} \times 8760\,\text{hours} = 7.22\,\text{Wh}
$$


$$
E_{\text{accelerometer}} = 0.000495\,\text{W} \times 8760\,\text{hours} = 4.33\,\text{Wh}
$$

$$
E_{\text{MQ9}} = 0.0099\,\text{W} \times 8760\,\text{hours} = 86.76\,\text{Wh}
$$

Then the total energy consumption for the 3 sensors in 1 year is:

$$
\text{Total Energy} = E_{\text{DHT11}} + E_{\text{accelerometer}} + E_{\text{MQ9}}
$$

$$
\text{Total Energy} = 7.22\,\text{Wh} + 4.33\,\text{Wh} + 86.76\,\text{Wh} = 98.31\,\text{Wh}
$$


Given two lithium ion batteries are available with capacities 9900 mAh and 6800 mAh respectively, their parallel connection will result in an algebraic sum of their capacities of 16700 mAh. Now we need to convert this total capacity from mAh to Wh as follows:

$$
\text{Battery Capacity (Wh)} = \frac{\text{Battery Capacity (mAh)} \times \text{Voltage (V)}}{1000} = \frac{16700 \, \text{mAh} \times 3.3 \, \text{V}}{1000} = \frac{55110}{1000} = 55.11 \, \text{Wh} 
$$

Then to estimate the battery lifetime:

$$
\text{Battery Lifetime (hours)} = \frac{55.11 \text{ Wh}}{98.31 \text{ Wh per year}} \times 8760 \text{ hours} \approx 4880 \text{ hours} 
$$

To convert hours into days divide by 24: 

$$
\text{Battery Lifetime (days)} = \frac{4880}{24} \approx 203.33 \text{ days} 
$$

Then it can be deduced that the power supply will last approximately **203 days**, about 6.7 months, which proves the system achieved the requirement of being able to run at least **6 months**.
 


## 5. Data Storage

### Required storage for one year

Since the data storage frequency is 5 seconds, the total data points or data entries will be calculated by dividing the number of seconds per day over the collection frequency 
  
  $$
  \frac{86400 \text{ seconds/day}}{5 \text{ seconds/entry}} = 17,280 \text{ entries/day}
  $$

 Since each cell or field approximately consumes 10 bytes, and there is a total of 7 fields per data entry [Sheet time, Timestamp, CO, Vibration, Humidity, Roll angle, Pitch angle], then the storage needed per data entry is 70 bytes.

$$
17,280 \text{ entries/day} \times 70 \text{ bytes/entry} = 1,209,600 \text{ bytes/day} \approx 1.15 \text{ MB/day}
$$

Then the needed storage for one year is:

$$
1.15 \text{ MB/day} \times 365 \text{ days} = 419.75 \text{ MB/year}
$$

which is provided by the large storage capacity in the cloud-based Google sheet.

### Data storage mechanism
 
 The following section explains how the data has been regularly logged from the ESP32 to a Google Spreadsheet using a Google Apps Script and HTTP POST requests


First, a Google Spreadsheet is created and a new Google Apps Script was accessed from clicking Extensions > Apps Script. The script is then programmed to handle HTTP POST requests, parse the JSON payload, and append the data to rows in the spreadsheet regularly. The script was deployed as a web app, and URL was provided that the ESP32 later used to send the data.
The code for the script is shown below can be accessed in its respective file in the repository.

The ESP32 code included the WiFi.h, HTTPClient.h, ArduinoJson.h libraries to connect to the WiFi network and periodically send the data from sensors to the Google Script URL. This is done with the server URL obtained from the deployment in the ESP32 code. 
The following code is a selected part from the main program code explaining the data sending process:

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Google Sheets Script Web App URL
const char* serverName = "https://script.google.com/macros/s/AKfycbxQk6eSnsTtMaqnAH6qPdIR_RvmXM1aUqoxLsdW7h6LN62MFMIkM_Q8Y4trZ-idLMjaug/exec"; // Replace with your Apps Script Web App URL

void sendDataToGoogleSheets(float coValue, float vibrationIntensity, float humidity, float roll, float pitch) {
  HTTPClient http;
  http.begin(serverName);

  // Prepare JSON payload
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["co"] = coValue;
  jsonDoc["vibration Intensity"] = vibrationIntensity;
  jsonDoc["Humidity"] = humidity;
  jsonDoc["Roll"] = roll;

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

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float coValue = 0.004135; // Example CO value
    float vibrationIntensity = 0.081657; // Example vibration value
    float humidity = 48; // Example humidity value
    float roll = 50.34708; // Example roll angle

    sendDataToGoogleSheets(coValue, vibrationIntensity, humidity, roll, pitch);
    delay(5000); // Send data every 5 seconds
  } else {
    Serial.println("WiFi not connected.");
  }
}

```

The `setup()` function initializes the WiFi connection, while the `sendData()` function constructs and sends the JSON payload to the Google Apps Script URL. 


## 6. Data visualization 

The sensor data was visualized using the Blynk platform. The Blynk library was installed in the Arduino IDE and the WiFi credentials were included. A new template was created in blynk and the template ID, device name, and authentication token were copied and defined in the program. The `loop()` function was used to send sensor data to Blynk using virtual pins.
This codeblock is an example code demonstrating setting the virtual pins and connecting with Blynk:

```cpp
#define BLYNK_TEMPLATE_ID "TMPL2eiVZOmcs"
#define BLYNK_TEMPLATE_NAME "ESPData"
#define BLYNK_AUTH_TOKEN "MupeH3xOrfox_UeyqfiBs9mFYLuHlMkC"

#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "11310";
char pass[] = "12312335";

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();
  
  float coValue = 0.004135; // Example CO value
  float vibrationIntensity = 0.081657; // Example vibration value
  float humidity = 48; // Example humidity value
  float roll = 50.34708; // Example roll angle

  Blynk.virtualWrite(CO, coValue);
  Blynk.virtualWrite(Vibration, vibrationIntensity);
  Blynk.virtualWrite(Humidity, humidity);
  Blynk.virtualWrite(Tiltrollangle, roll);

  delay(5000); // Send data every 5 seconds
}
}
```

Each chart was assigned a datastream from the virtual pin that is connected to each value of the sensor readings.
This solution made the data be updated real-time visually for decision making.
