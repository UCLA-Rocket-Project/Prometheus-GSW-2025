To set up MQTT on laptop:
https://bhave.sh/micropython-mqtt/
- run ```mosquitto -c /opt/homebrew/etc/mosquitto/mosquitto.conf```
- Note: mosquitto.conf itself must be modified. add this to the end of the file
 ```listener 1883 allow_anonymous true```

To publish to MQTT broker
- publishToMQTTBrokerOverWifi.ino

https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
https://pubsubclient.knolleary.net/api

To subscribe to MQTT broker 
https://medium.com/@potekh.anastasia/a-beginners-guide-to-mqtt-understanding-mqtt-mosquitto-broker-and-paho-python-mqtt-client-990822274923
- subscribeToMQTTBroker.py
- ```python -m venv venv```
- ```source venv/bin/activate```
- ```pip install paho-mqtt```
