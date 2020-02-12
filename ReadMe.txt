MASTER:-------------------------------------------------
Can work without wifi (NO MQTT)
Recieve information in JSON format
Send information in JSON Format
If the recieved Key "message" is not "hello" then will send the message to the MQTT Broker
Wifi credentials and MQTT credentials can be changed in code
Always listening for new incoming messages except when send information over MQTT


SLAVE---------------------------------------------------
At sart always send "hello" message to the master. this can be changed to test the MQTT on master on line 102 of "espnow.c"
Recieve information in JSON format
Send information in JSON Format
Deepsleep enabled each time that send the second message (type, data, deviceId)
Deepsleep is set to 1 min
Key "data" is a random number between 1 - 4000