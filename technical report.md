## Technical Report on Power Consumption Estimation for ESP32 with Selected Sensors

### Introduction
This report provides a detailed methodology to estimate the power consumption of an ESP32 microcontroller connected to various sensors, specifically a DHT11 temperature and humidity sensor, an MQ-9 gas sensor, and an accelerometer. The objective is to assess the feasibility of operating this system continuously for six months using three 9900 mAh batteries connected in parallel.

### 1. Sensor Power Consumption Overview
The first step involves identifying the power consumption characteristics of each sensor:

- **DHT11 Temperature and Humidity Sensor**:
  - **Active Current**: Up to 2.5 mA during measurement.
  - **Standby Current**: Approximately 50 µA when idle[2][6].

- **MQ-9 Gas Sensor**:
  - **Active Current**: Typically around 150 mA during heating, but this occurs for a brief period (approximately 10 seconds per hour).
  
- **Accelerometer (e.g., ADXL345)**:
  - **Active Current**: Approximately 23 µA in measurement mode.
  - **Low Power Mode**: Can be reduced to about 0.1 µA when in sleep mode.

### 2. Active Time Calculation
Next, we need to calculate how long each sensor is active over a day. For this example:

- **DHT11**: 
  - Active for one reading every minute (100 ms per reading):
    $$
    \text{Active Time} = 1440 \text{ minutes/day} \times 0.1 \text{ seconds} = 144 \text{ seconds/day}
    $$

- **MQ-9**:
  - Active for heating once every hour for 10 seconds:
    $$
    \text{Active Time} = 24 \text{ readings} \times 10 \text{ seconds} = 240 \text{ seconds/day}
    $$

- **Accelerometer**:
  - Assuming it takes a reading every second (1 second per reading):
    $$
    \text{Active Time} = 86400 \text{ seconds/day}
    $$

### 3. Daily Power Consumption Estimation
We can now estimate the daily power consumption for each sensor using the formula:

$$
\text{Daily Power Consumption (mAh)} = \left(\frac{\text{Current (mA)} \times \text{Active Time (seconds)}}{3600}\right)
$$

Calculating each sensor's consumption:

- **DHT11**:
    - 
    $$
    = \left(\frac{2.5\,\text{mA} \times 144\,\text{s}}{3600}\right) = 0.10\,\text{mAh}
    $$

- **MQ-9**:
    - 
    $$
    = \left(\frac{150\,\text{mA} \times 240\,\text{s}}{3600}\right) = 10\,\text{mAh}
    $$

- **Accelerometer**:
    - 
    $$
    = \left(\frac{23\,\mu\text{A} \times 86400\,\text{s}}{3600}\right) = 0.46\,\text{mAh}
    $$

### Total Daily Consumption
Summing these values provides the total daily power consumption:

$$
\text{Total Daily Consumption} = 0.10 + 10 + 0.46 = 10.56\,\text{mAh}
$$

### Battery Capacity Calculation
With three batteries of capacity $$9900\,mAh$$ connected in parallel, the total capacity is:

$$
\text{Total Capacity} = 3 \times 9900\,mAh = 29700\,mAh
$$

To determine how long these batteries can sustain the system with an estimated daily consumption of $$10.56\,mAh$$:

1. Total operational days without replacement:

$$
\text{Days of Operation} = \frac{\text{Total Capacity}}{\text{Daily Consumption}} = \frac{29700}{10.56} \approx 2816\,days
$$

### Conclusion
This technical report demonstrates a structured approach to estimating power consumption for an ESP32-based system utilizing a DHT11 sensor, MQ-9 gas sensor, and an accelerometer. The calculations indicate that with efficient management of active times and sleep modes, the system can operate continuously for an extended period—far exceeding the six-month target—using three parallel-connected batteries with a total capacity of $$29700\,mAh$$. This analysis underscores the importance of optimizing sensor usage and understanding their power characteristics in battery-operated applications.
