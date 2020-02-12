# ESP32-EspNow
Communication over Esp Now protocol

One master and one slave.
Master is waiting for "key message" in JSON format (Slave message)
When the key message is recieved by Master, it response with another "key message" in JSON format
Slave recieve this new "key message" and start generating random data simulating a sensor measurement
and it send it to master.
Master connect to a wifi network and post the message to a MQTT server
