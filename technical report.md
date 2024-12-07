Alright, let's create a customized structure for your mine safety technical report and include sample data, graphs, and tables. Here's a detailed example:

---

### **Smart Mine Safety System: Technical Report**

#### **Title Page**
- **Project Title:** Smart Mine Safety System
- **Team Members:** [Names of the three students]
- **Date of Submission:** [Date]

#### **Executive Summary**
- A brief summary of the project, its objectives, key findings, and outcomes.
  - The Smart Mine Safety System integrates accelerometer, MQ-9 gas sensor, and (load cell/temperature sensor) to monitor mine conditions.
  - The system provides real-time data, alerts, and visualizations to enhance mine safety.

#### **Table of Contents**
1. Introduction
2. System Design
   - Sensor Selection and Integration
   - Signal Processing
   - Real-Time Monitoring and Alerts
3. Data Visualization
4. Power Efficiency
5. Data Storage
6. Testing and Results
7. Conclusion
8. References
9. Appendices

#### **1. Introduction**
- Background of the project
- Purpose and scope
- Overview of mine safety challenges being addressed

## 2. Sensor integration

##### **Sensor Selection and Integration**
- **Sensors Used:**
  - **Accelerometer:** Measures vibration and motion.
  - **MQ-9 Gas Sensor:** Monitors gas levels (e.g., CO).
  - **Load Cell/Temperature Sensor Placeholder:** Monitors weight/temperature.
- **Calibration Data:**
  - Accelerometer:
    ```markdown
    | Vibration Intensity (m/s²) | Raw Data (mV) |
    |----------------------------|---------------|
    | 0                          | 500           |
    | 5                          | 1500          |
    | 10                         | 2500          |
    ```
  - MQ-9 Gas Sensor:
    ```markdown
    | Gas Concentration (ppm) | Raw Data (mV) |
    |-------------------------|---------------|
    | 0                       | 400           |
    | 25                      | 1200          |
    | 50                      | 2000          |
    ```
  - Load Cell (Placeholder):
    ```markdown
    | Load (kg) | Raw Data (mV) |
    |-----------|---------------|
    | 0         | 1000          |
    | 50        | 2000          |
    | 100       | 3000          |
    ```
  - Temperature Sensor (Placeholder):
    ```markdown
    | Temperature (°C) | Raw Data (mV) |
    |------------------|---------------|
    | 0                | 100           |
    | 25               | 500           |
    | 50               | 900           |
    ```
- **Integration Process:**
  - Detailed description of how sensors were integrated into the system.

##### **Signal Processing**
- **Algorithms Used:**
  - Description of algorithms to convert raw data into meaningful metrics (e.g., calculating vibration intensity, gas concentration).
- **Calibration Techniques:**
  - Example: Using linear regression to map sensor data to physical quantities.

##### **Real-Time Monitoring and Alerts**
- **Alert System:**
  - Red LED for vibration > 5 m/s².
  - Warning message for gas concentration > 25 ppm.
- **Monitoring:**
  - System provides updates every 30 seconds.

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


#### **3. Data Visualization**
- **Dashboard Design:**
  - Overview of Blynk dashboard setup.
  - Visualization of historical trends and real-time conditions.
  - Example graph:
    ```markdown
    ![Example Graph](https://via.placeholder.com/600x300?text=Example+Graph)
    ```
  - **Hypothetical Graphs:**
    - **Vibration Intensity:**
      ```markdown
      | Time (s) | Vibration Intensity (m/s²) |
      |----------|----------------------------|
      | 0        | 0                          |
      | 30       | 3                          |
      | 60       | 7                          |
      ```
    - **Gas Concentration:**
      ```markdown
      | Time (s) | Gas Concentration (ppm) |
      |----------|-------------------------|
      | 0        | 0                       |
      | 30       | 15                      |
      | 60       | 30                      |
      ```
- **Error Bars:**
  - Indications of measurement error for each data point.

## 4. Power Efficiency
 #### **4. Power Efficiency**

A power supplying system is to be designed so that the sensors can operate for 6 continuous months without battery replacement. The following section will discuss the power consumption calculation to choose a battery with a suitable rating.

First, the datasheets were referenced to obtain the maximum current consumption of each sensor, voltage, and sampling rate:

| **Sensor**                  | **Max Operating Current** | **Operating Voltage** | **Sampling Rate**                 |
| --------------------------- | ------------------------- | --------------------- | --------------------------------- |
| **DHT11**                   | Up to **2.5 mA**          | **3 to 5.5 V**        | **1 Hz (1 reading/second)**       |
| **MQ-9**                    | Up to **150 mA**          | **5 V**               | **1 reading/hour (10s)**          |
| **Accelerometer (ADXL345)** | Up to **23 µA**           | **2.5 to 5.25 V**     | **Continuous (1 reading/second)** |

Then, the active time was calculated as follows for each sensor over a day:

$$
  \text{Active Time} = \text{Number of Readings} \times \text{Duration per Reading}
$$

For the DHT11 humidity sensor, it takes one reading per minute with each reading lasting 100 milliseconds, so the calculated active time was:

$$
  \text{Active Time} = 1440 \text{ minutes/day} \times 0.1 \text{ seconds} = 144 \text{ seconds/day}
$$

For the MQ-9 sensor, given it has a heating mode that repeats itself and lasts about 10 seconds each hour and consumes the most current in this phase, the active time was:

$$
    \text{Active Time} = 24 \text{ readings} \times 10 \text{ seconds} = 240 \text{ seconds/day}
$$
   
For the Accelerometer that takes a reading every second with 1 second per reading, the active time was:

$$
    \text{Active Time} = 86400 \text{ seconds/day}
$$

Then, we estimated the daily power consumption for each sensor using the formula:

$$
\text{Daily Power Consumption (mAh)} = \left(\frac{\text{Current (mA)} \times \text{Active Time (seconds)}}{3600}\right)
$$

- **DHT11**: 
    
    $$ \left(\frac{2.5\,\text{mA} \times 144\,\text{s}}{3600}\right) = 0.10\,\text{mAh} $$
    
- **MQ-9**:
    
    $$ \left(\frac{150\,\text{mA} \times 240\,\text{s}}{3600}\right) = 10\,\text{mAh} $$

- **Accelerometer**:
    
    $$ \left(\frac{23\,\mu\text{A} \times 86400\,\text{s}}{3600}\right) = 0.55\,\text{mAh} $$

So, the total daily consumption is the summation of these values:

$$
\text{Total Daily Consumption} = 0.10 + 10 + 0.55 = 10.65\,\text{mAh}
$$

Given two lithium ion batteries with the same voltage and different capacities of 9900 and 6800 mAh, connecting them in parallel will result in the summation of their capacities with a total capacity of:

$$
\text{Total Capacity} = \text{Capacity of Battery 1} + \text{Capacity of Battery 2} = 9900\,\text{mAh} + 6800\,\text{mAh} = 16700\,\text{mAh}
$$

To find out the battery lifetime in hours for which the system will operate continuously, we divide the total capacity over the average power consumption:

$$
\text{Battery Lifetime (hours)} = \frac{\text{Battery Capacity (mAh)}}{\text{Average Power Consumption (mA)}} = \frac{16700\,\text{mAh}}{10.65\,\text{mAh/day}} \times 24 = 37,644 \,\text{hours}
$$

We previously calculated that the total energy consumption for the system over 1 year is 98.31 Wh. Now, we need to calculate how long the battery will last based on this total energy consumption.

Step 1: Convert the battery capacity to Wh

Assuming the battery voltage is 3.3V (since the system operates at 3.3V), we can convert the 16700 mAh capacity into watt-hours (Wh):

$$
\text{Battery Capacity (Wh)} = \frac{\text{Battery Capacity (mAh)} \times \text{Voltage (V)}}{1000} = \frac{16700 \times 3.3}{1000} = 55.11 \text{ Wh}
$$

Step 2: Estimate battery lifetime

Now, let's calculate the lifetime of the battery. The total energy consumption for the system is 38.88 Wh, and the battery has a total capacity of 55.11 Wh.

$$
\text{Battery Lifetime (hours)} = \frac{55.11 \text{ Wh}}{38.88 \text{ Wh per year}} \times 8760 \text{ hours} = \frac{55.11}{38.88} \times 8760 \approx 12409.86 \text{ hours}
$$

$$
\text{Battery Lifetime (days)} = \frac{12409.86}{24} \approx 517.08 \text{ days}
$$

 
 
 
 


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
 The sensor data was sent from the ESP32 to an external google sheet that updates every 5 seconds, 

#### **6. Testing and Results**
- **System Performance:**
  - Summary of tests conducted.
  - Placeholder for specific test results related to load cell/temperature sensor.

#### **7. Conclusion**
- Summary of findings.
- Implications and potential impact.
- Suggestions for future improvements.

#### **8. References**
- List of all references and resources used.

#### **9. Appendices**
- Additional data, charts, or diagrams supporting the main text.
- Data Sheets: Specific parts from sensor data sheets showing calibration.

