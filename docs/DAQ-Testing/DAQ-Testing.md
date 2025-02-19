# Testing of DAQ

## Conclusions
1. Reading from the sensors is not a bottleneck
    - Takes a couple of milliseconds to read from each sensor
        - the fact that it takes 9 ms to read from a pt and 151 to read from all pts may be a sign that waiting for a reading from a sensor blocks reading from other sensors, but even if that were true, the delay is in milliseconds, which is inconsequential

2. Reading from Serial is the bottleneck
    - The delay in data transmission for the DAQ is set at 200ms, which equates to readings of 5hz
    - I should have measured this, but just by eyeballing it, the rate of data transfer to the laptop is much slower than 5hz. 
        - This can be supplemented by comparing the rate at which the new lines are printed with the Grafana Code, against a python loop that prints a new line every 200ms
            ```
            import time

            while True:
                time.sleep(2 * 10**-3)
                print("HI")
            ```

## Data
1. Reading and passing sensor values (DAQ transmitter)
    ```
    Time spent printing to Serial: : 123
    Time spent to print to rs485Serial: : 0
    Time spent reading from pt 1: : 9
    Time spent reading from pt 2: : 9
    Time spent reading from pt 3: : 10
    Time spent reading from all pts: : 151
    Time spent reading from LC: : 0
    Time spent creating storeStr: : 0
    Asensorvals pt1=950.18,pt2=-115.76,pt3=-109.53,pt4=0.00,pt5=0.00,pt6=0.00,lc1=14327,lc2=0.00Z
    Time spent printing to Serial: : 1
    Time spent to print to rs485Serial: : 0
    Time spent reading from pt 1: : 9
    Time spent reading from pt 2: : 9
    Time spent reading from pt 3: : 10
    Time spent reading from all pts: : 28
    Time spent reading from LC: : 0
    Time spent creating storeStr: : 1
    Asensorvals pt1=950.18,pt2=-115.76,pt3=-109.53,pt4=0.00,pt5=0.00,pt6=0.00,lc1=18208,lc2=0.00Z
    ```
2. Receiving and transmitting of sensor values
    ```
    Time spent reading from RS485S: 2
    Asensorvals pt1=949.04,pt2=-115.76,pt3=-109.53,pt4=0.00,pt5=0.00,pt6=0.00,lc1=31859,lc2=0.00Z
    Time spent printing to serial: 0
    Time spent reading from RS485S: 2
    Asensorvals pt1=949.04,pt2=-115.76,pt3=-109.53,pt4=0.00,pt5=0.00,pt6=0.00,lc1=31749,lc2=0.00Z
    Time spent printing to serial: 0
    Time spent reading from RS485S: 2
    Asensorvals pt1=949.04,pt2=-115.76,pt3=-109.53,pt4=0.00,pt5=0.00,pt6=0.00,lc1=31723,lc2=0.00Z
    Time spent printing to serial: 0
    Time spent reading from RS485S: 2
    Asensorvals pt1=949.04,pt2=-115.76,pt3=-109.53,pt4=0.00,pt5=0.00,pt6=0.00,lc1=31612,lc2=0.00Z
    Time spent printing to serial: 0
    ```

3. Flame graph of grafana code
![Python-Grafana-Flame-Graph](./readings_without_timeout_2.svg?)