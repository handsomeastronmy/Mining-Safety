#### **4. Power Efficiency**
 A power supplying system is to be designed, so that the sensors can operate for 6 continuous months without battery replacement. So the following section will discuss the power consumption calculation to choose a battery with a suitable rating.
 
 First, the datasheets were referenced to obtain the maximum current consumption of each sensor, voltage and sampling rate:

| **Sensor**                  | **Max Operating Current** | **Operating Voltage** | **Sampling Rate**                 |
| --------------------------- | ------------------------- | --------------------- | --------------------------------- |
| **DHT11**                   | Up to **2.5 mA**          | **3 to 5.5 V**        | **1 Hz (1 reading/second)**       |
| **MQ-9**                    | Up to **150 mA**          | **5 V**               | **1 reading/hour (10s)**          |
| **Accelerometer (ADXL345)** | Up to **23 µA**           | **2.5 to 5.25 V**     | **Continuous (1 reading/second)** |

Then, the active time was calculated as follows for each sensor over a day:

$$
  \text{Active Time} = \text{Number of Readings} \times \text{Duration per Reading}
$$
  For the DHT11 humidity sensor, it takes one reading per minute with each reading lasting 100 milliseconds so the calculated active time was:
  
  $$
  \text{Active Time} = 1440 \text{ minutes/day} \times 0.1 \text{ seconds} = 144 \text{ seconds/day}
  $$

 For the MQ-9 sensor, given it has a heating mode that repeats itself and lasts about 10 seconds each hour and consumes the most current in this phase:

 $$
    \text{Active Time} = 24 \text{ readings} \times 10 \text{ seconds} = 240 \text{ seconds/day}
 $$
   
For the Accelerometer that takes a reading every second with 1 second per reading
    $$
    \text{Active Time} = 86400 \text{ seconds/day}
    $$

Then ,we estimated the daily power consumption for each sensor using the formula:

$$
\text{Daily Power Consumption (mAh)} = \left(\frac{\text{Current (mA)} \times \text{Active Time (seconds)}}{3600}\right)
$$



- **DHT11**:
    
$$
    = \left(\frac{2.5\,\text{mA} \times 144\,\text{s}}{3600}\right) = 0.10\,\text{mAh} 
$$

- **MQ-9**:
    
$$
    = \left(\frac{150\,\text{mA} \times 240\,\text{s}}{3600}\right) =
    10\,\text{mAh}
$$

- **Accelerometer**:

$$
    = \left(\frac{23\,\mu\text{A} \times 86400\,\text{s}}{3600}\right) = 0.46\,\text{mAh}    
$$

So the total daily consumption is the summation of these values: 

$$
\text{Total Daily Consumption} = 0.10 + 10 + 0.46 = 10.56\,\text{mAh}
$$

Given two lithium ion batteries with the same voltage and different capacities of 9900 and 6800 mAh, connecting them in parallel will result in the summation of their capacities with a total capacity of: 
   
   $$
   \text{Total Capacity} = \text{Capacity of Battery 1} + \text{Capacity of Battery 2}
   $$

   $$
   \text{Total Capacity} = 9900\,\text{mAh} + 6800\,\text{mAh} = 16700\,\text{mAh}
   $$

To find out the time for which the system will operate continuously we divide the total capacity over the daily consumption:

$$
\text{Days of Operation} = \frac{\text{Total Capacity}}{\text{Daily Consumption}}
$$

Substituting in our values:

$$
\text{Days of Operation} = \frac{16700\,\text{mAh}}{10.56\,\text{mAh}} \approx 1583\,\text{days}
$$

Which proves that the system can run efficiently for at least 6 months.

