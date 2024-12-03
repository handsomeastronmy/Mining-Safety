## 4. Power Efficiency

A power supplying system is to be designed so that the sensors can operate for 6 continuous months without battery replacement. The following section will discuss the power consumption calculation to choose a battery with a suitable rating.

First, the datasheets were referenced to obtain the maximum current consumption of each sensor, voltage, and sampling rate:

\[
\begin{array}{|l|c|c|c|}
\hline
\text{Sensor} & \text{Max Operating Current} & \text{Operating Voltage} & \text{Sampling Rate} \\
\hline
\text{DHT11} & \text{Up to } 2.5\, \text{mA} & 3 \text{ to } 5.5\, \text{V} & 1\, \text{Hz (1 reading/second)} \\
\hline
\text{MQ-9} & \text{Up to } 150\, \text{mA} & 5\, \text{V} & 1\, \text{reading/hour (10s)} \\
\hline
\text{Accelerometer (ADXL345)} & \text{Up to } 23\, \mu\text{A} & 2.5 \text{ to } 5.25\, \text{V} & \text{Continuous (1 reading/second)} \\
\hline
\end{array}
\]

Then, the active time was calculated as follows for each sensor over a day:

\[
\text{Active Time} = \text{Number of Readings} \times \text{Duration per Reading}
\]

For the DHT11 humidity sensor, it takes one reading per minute with each reading lasting 100 milliseconds, so the calculated active time was:
\[
\text{Active Time} = 1440\, \text{minutes/day} \times 0.1\, \text{seconds} = 144\, \text{seconds/day}
\]

For the MQ-9 sensor, given it has a heating mode that lasts about 10 seconds each hour:
\[
\text{Active Time} = 24\, \text{readings} \times 10\, \text{seconds} = 240\, \text{seconds/day}
\]

For the Accelerometer that takes a reading every second:
\[
\text{Active Time} = 86400\, \text{seconds/day}
\]

Next, we estimated the daily power consumption for each sensor using the formula:

\[
\text{Daily Power Consumption (mAh)} = \left(\frac{\text{Current (mA)} \times \text{Active Time (seconds)}}{3600}\right)
\]

Calculating each sensor's consumption:

- **DHT11**:
    \[
    = \left(\frac{2.5\,\text{mA} \times 144\,\text{s}}{3600}\right) = 0.10\,\text{mAh}
    \]

- **MQ-9**:
    \[
    = \left(\frac{150\,\text{mA} \times 240\,\text{s}}{3600}\right) = 10\,\text{mAh}
    \]

- **Accelerometer**:
    \[
    = \left(\frac{23\,\mu\text{A} \times 86400\,\text{s}}{3600}\right) = 0.46\,\text{mAh}
    \]

So the total daily consumption is the summation of these values: 
\[
\text{Total Daily Consumption} = 0.10 + 10 + 0.46 = 10.56\,\text{mAh}
\]

Given two lithium-ion batteries with capacities of 9900 mAh and 6800 mAh, connecting them in parallel will result in a total capacity of: 
\[
\begin{align*}
\text{Total Capacity} &= \text{Capacity of Battery 1} + \text{Capacity of Battery 2}\\
&= 9900\,\text{mAh} + 6800\,\text{mAh} = 16700\,\text{mAh}
\end{align*}
\]

To find out how long the system will operate continuously, we divide the total capacity by the daily consumption:

Days of Operation Calculation Formula:
\[
\begin{align*}
\text{Days of Operation} &= \frac{\text{Total Capacity}}{\text{Daily Consumption}}\\
&= \frac{16700\,\text{mAh}}{10.56\,\text{mAh}} \approx 1583\,days
\end{align*}
\]

This proves that the system can run efficiently for at least six months.
