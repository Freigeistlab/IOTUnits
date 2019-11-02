## Sourcecode for ESP8266 NodeMCU-lolin v3 based hardware units
<br/>

### LED unit
![LED unit](https://user-images.githubusercontent.com/10089188/68074697-2ec4a000-fd5b-11e9-8cdb-ec4f10e9d393.png)

### Ultrasound based distance unit
![Ultrasound based distance unit](https://user-images.githubusercontent.com/10089188/68074703-48fe7e00-fd5b-11e9-8789-79fedbf4add1.png)

### Rotary encoder unit
![Rotary encoder unit](https://user-images.githubusercontent.com/10089188/68074708-5ca9e480-fd5b-11e9-8ab3-7d5f0ff902cc.png)

### Scale unit (sense when tape is teared)
![Scale unit (sense when tape is teared)](https://user-images.githubusercontent.com/10089188/68074720-7d723a00-fd5b-11e9-93ad-4a4eb1c00376.png)

### Button unit
![Button unit](https://user-images.githubusercontent.com/10089188/68074729-a266ad00-fd5b-11e9-8800-b125dc71d820.png)


### API calls

Unit Name | Route | Data | Return | Description
------------ | ------------- | ------------- | ------------- | -------------
all | / | {ip:String, port:String} | Code:200, {UNIT_NAME:String, ID:String} | Units will listen on UNIT_PORT until they receive server IP and port for sending values
LEDUnit | /set | {index:String, r:String, g:String, b:String} | Code:200, String | set LED at index to color {r,g,b}
LEDUnit | /set | {r:String, g:String, b:String} | Code:200, String | set all LEDS to color {r,g,b}
LEDUnit | /handleConfigure | n/a | Code:200, String | not implemented, does nothing
buttonUnit, distanceUnit, encoderUnit, lightSensUnit, scaleUnit | /handleConfigure | {end} | Code:200, String | shut down server, no more routes accessible (does not stop unit from sending sensor readings)
distanceUnit, encoderUnit, lightSensUnit, scaleUnit | /handleConfigure | {threshold:String, sensitivity:String, mode:String} | Code:200, String | set threshold, sensitivity and mode

<br/><br/>

### Modes

Modenumber | Modename | Description
------------ | ------------- | -------------
0 | BOOLEAN | send updates when sensor reading crosses threshold (rising and falling flank)
1 | CONTINOUS | send updates when sensor reading changes by more than sensitivity
2 | IDLE | do not send updates

<br/><br/>

### Defaults

Unit Name | Parameters | Default
------------ | ------------- | -------------
all | UNIT_PORT | 43432
distanceUnit | mode, sensitivity, threshold | CONTINOUS, 1, 100
encoderUnit | mode, sensitivity, threshold | CONTINOUS, 100, 1000
lightSensUnit | mode, sensitivity, threshold | BOOLEAN, 5, 50
scaleUnit | mode, sensitivity, threshold | CONTINOUS, 10, 1000

