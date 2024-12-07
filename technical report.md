
## 1. Introduction

## 2. Sensor integration
The sensors connections were planned in the Cirkit Designer Software and can be seen in the following figure:
**![](https://lh7-rt.googleusercontent.com/slidesz/AGV_vUfpIIhmLG60DCN_r03ort3DBUqlsi1_eMqJrW-7dWEWY1xa75wYNkm4i9utNh_CxpffEariiI11QVOV7fsfx2c2GiPXY2wcjF_oi_b1JPsGrKyqwjHZiDq-nO8ffOKdcDKABTC2N8qfCDD-LCQV4CE=s2048?key=XSWHYWdthl-CbACJd_wcfhjO)**

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



   ```cpp
   float roll = atan2(event.acceleration.y, event.acceleration.z) * 180.0 / PI;
   float pitch = atan2(-event.acceleration.x, sqrt(event.acceleration.y * event.acceleration.y + event.acceleration.z * event.acceleration.z)) * 180.0 / PI;
   ```

     - **Roll**: Uses the Y and Z axes to determine the tilt angle around the X-axis.
     - **Pitch**: Uses the X axis and the magnitude of Y and Z axes to determine the tilt angle around the Y-axis.
   - **Mathematical Explanation**:
     - `atan2(y, z)` calculates the angle between the projection of the vector on the Y-Z plane and the Z axis, providing the roll angle.
     - `atan2(-x, sqrt(y^2 + z^2))` calculates the angle between the projection on the X axis and the vector's magnitude in the Y-Z plane, giving the pitch angle.


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
The code for the script is shown below:

```javascript
function doPost(e) {
  var ss = SpreadsheetApp.openByUrl('YOUR_SPREADSHEET_URL');
  var sheet = ss.getSheetByName('Sheet1');
  var data = JSON.parse(e.postData.contents);
  sheet.appendRow([new Date(), data.CO, data.Vibration, data.Humidity, data.Roll, data.Pitch]);
  return ContentService.createTextOutput('Data stored successfully');
}
```


The ESP32 code included the WiFi.h, HTTPClient.h, ArduinoJson.h libraries to connect to the WiFi network and periodically send the data from sensors to the Google Script URL. This is done the server URL obtained from the deployment in the ESP32 code. 
The following code is a selected part from the main program code explaining the data sending process:

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* serverName = "YOUR_WEB_APP_URL"; // Google Apps Script URL

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void sendData(float co, float vibration, float humidity, float roll, float pitch) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["CO"] = co;
    jsonDoc["Vibration"] = vibration;
    jsonDoc["Humidity"] = humidity;
    jsonDoc["Roll"] = roll;
    jsonDoc["Pitch"] = pitch;
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    int httpResponseCode = http.POST(jsonString);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void loop() {
  float co = 0.004135; // Example CO value
  float vibration = 0.081657; // Example vibration value
  float humidity = 48; // Example humidity value
  float roll = 50.34708; // Example roll angle
  float pitch = 11.94512; // Example pitch angle
  sendData(co, vibration, humidity, roll, pitch);
  delay(5000); // Send data every 5 seconds
}
```

The `setup()` function initializes the WiFi connection, while the `sendData()` function constructs and sends the JSON payload to the Google Apps Script URL. 


## 6. Data visualization 

The sensor data was visualized using the Blynk platform. The Blynk library was installed in the Arduino IDE and the WiFi credentials were included. A new template was created in blynk and the template ID, device name, and authentication token were copied and defined in the program. The `loop()` function was used to send sensor data to Blynk using virtual pins.
This codeblock is an example code demonstrating setting the virtual pins and connecting with Blynk:

```cpp
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_DEVICE_NAME "YOUR_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_PASSWORD";

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();
  float co = 0.004135; // Example CO value
  float vibration = 0.081657; // Example vibration value
  float humidity = 48; // Example humidity value
  float roll = 50.34708; // Example roll angle
  float pitch = 11.94512; // Example pitch angle

  Blynk.virtualWrite(V1, co);
  Blynk.virtualWrite(V2, vibration);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V4, roll);
  Blynk.virtualWrite(V5, pitch);

  delay(5000); // Send data every 5 seconds
}
```

In the Blynk website multiple widget charts and labels were added to display the sensor data. 
The dashboard can be seen in the figure:

![](https://i.imgur.com/XKSu2A9.png)

Each chart was assigned a datastream from the virtual pin that is connected to each value of the sensor readings.
This solution made the data be updated real-time visually for decision making.

## 7. System performance in the test environment 


| **Timestamp**      | **Reading Time** | **Humidity** |
| ------------------ | ---------------- | ------------ |
| 12/2/2024 16:23:55 | 23:55            | 57           |
| 12/2/2024 16:23:56 | 23:55            | 57           |
| 12/2/2024 16:24:52 | 24:51:00         | 57           |
| 12/2/2024 16:25:02 | 25:1             | 58           |
| 12/2/2024 16:51:44 | 51:43:00         | 52           |
| 12/2/2024 16:51:49 | 51:49:00         | 61           |
| 12/2/2024 16:51:55 | 51:54:00         | 63           |
| 12/2/2024 16:52:00 | 51:59:00         | 64           |
| 12/2/2024 16:52:05 | 52:5             | 65           |
| 12/2/2024 16:52:11 | 52:11:00         | 66           |
| 12/2/2024 16:52:17 | 52:16:00         | 66           |
| 12/2/2024 16:52:22 | 52:21:00         | 67           |
| 12/2/2024 16:52:29 | 52:29:00         | 68           |
| 12/2/2024 16:52:34 | 52:34:00         | 69           |
| 12/2/2024 16:52:40 | 52:39:00         | 71           |
| 12/2/2024 16:52:45 | 52:45:00         | 72           |
| 12/2/2024 16:52:51 | 52:50:00         | 73           |
| 12/2/2024 16:52:57 | 52:56:00         | 74           |
| 12/2/2024 16:53:14 | 53:14:00         | 77           |
| 12/2/2024 16:53:23 | 53:23:00         | 77           |
| 12/2/2024 16:53:28 | 53:28:00         | 77           |
| 12/2/2024 16:53:33 | 53:33:00         | 77           |
| 12/2/2024 16:53:40 | 53:39:00         | 78           |
| 12/2/2024 16:53:45 | 53:45:00         | 79           |
| 12/2/2024 16:53:50 | 53:50:00         | 79           |
| 12/2/2024 16:53:56 | 53:56:00         | 80           |
| 12/2/2024 16:54:02 | 54:1             | 80           |
| 12/2/2024 16:54:07 | 54:6             | 80           |
| 12/2/2024 16:54:14 | 54:13:00         | 80           |
| 12/2/2024 16:54:19 | 54:19:00         | 80           |
| 12/2/2024 16:54:26 | 54:26:00         | 80           |
| 12/2/2024 16:54:33 | 54:32:00         | 80           |
| 12/2/2024 16:54:38 | 54:38:00         | 80           |
| 12/2/2024 16:54:44 | 54:43:00         | 81           |
| 12/2/2024 16:54:49 | 54:48:00         | 81           |
| 12/2/2024 16:54:54 | 54:53:00         | 81           |
| 12/2/2024 16:54:59 | 54:58:00         | 81           |
| 12/2/2024 16:55:04 | 55:4             | 81           |
| 12/2/2024 16:55:10 | 55:9             | 81           |
| 12/2/2024 16:55:17 | 55:16:00         | 81           |
| 12/2/2024 16:55:22 | 55:21:00         | 80           |
| 12/2/2024 16:55:29 | 55:28:00         | 80           |
| 12/2/2024 16:55:34 | 55:34:00         | 80           |
| 12/2/2024 16:55:43 | 55:42:00         | 80           |
| 12/2/2024 16:55:49 | 55:48:00         | 81           |
| 12/2/2024 16:55:54 | 55:54:00         | 81           |
| 12/2/2024 16:56:00 | 56:0             | 81           |
| 12/2/2024 16:56:06 | 56:6             | 80           |
| 12/2/2024 16:56:12 | 56:12:00         | 80           |
| 12/2/2024 16:56:22 | 56:21:00         | 80           |
| 12/2/2024 16:56:29 | 56:29:00         | 81           |
| 12/2/2024 16:56:36 | 56:35:00         | 82           |
| 12/2/2024 16:56:42 | 56:42:00         | 84           |
| 12/2/2024 16:56:48 | 56:48:00         | 86           |
| 12/2/2024 16:57:04 | 57:4             | 88           |
| 12/2/2024 16:57:06 | 57:5             | 89           |
| 12/2/2024 16:57:18 | 57:17:00         | 90           |
| 12/2/2024 16:57:24 | 57:23:00         | 91           |
| 12/2/2024 16:57:30 | 57:29:00         | 93           |
| 12/2/2024 16:57:35 | 57:34:00         | 94           |
| 12/2/2024 16:57:40 | 57:39:00         | 94           |
| 12/2/2024 16:57:45 | 57:45:00         | 94           |
| 12/2/2024 16:57:51 | 57:50:00         | 94           |
| 12/2/2024 16:57:56 | 57:56:00         | 94           |
| 12/2/2024 16:58:05 | 58:4             | 95           |
| 12/2/2024 16:58:10 | 58:9             | 95           |
| 12/2/2024 16:58:15 | 58:15:00         | 96           |
| 12/2/2024 16:58:21 | 58:20:00         | 96           |
| 12/2/2024 16:58:32 | 58:32:00         | 96           |
| 12/2/2024 16:58:45 | 58:44:00         | 97           |
| 12/2/2024 16:58:51 | 58:50:00         | 97           |
| 12/2/2024 16:59:00 | 59:0             | 97           |
|                    |                  |              |
