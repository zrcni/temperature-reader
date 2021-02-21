Contains two binaries: **read_sensor** and **thermo**.

### read_sensor.c
Reads from DHT22 sensor. Temperature and humidity data.   
Raspberry Pi's pin number is provided as the first argument to the built binary: `./read_sensor 7`
Prints "temp=%.2f,hum=%.2f" to stdout.

### conditions.c
Lib file for thermo.c   
Uses **read_sensor** binary to read from the sensor and puts the data into a struct.

### thermo.c
Main file   
Gets the DHT22 sensor data via **get_conditions** function in conditions.c and it them to an MQTT broker.
Can be configured to read and publish once or continuously at an interval.

TODO: detect sensor and fail if it doesn't exist

### job.sh
At the moment this is used to run the program in the background on a Raspberry Pi. Later I'll probably set up something to schedule the program execution.

```
./bin/thermo --deviceid <device-id> --pin <wiringpi-pin-number> --interval <seconds>
```

Currently compiling the C programs on the Raspberry Pi (super slowly), because I haven't figured out how to cross-compile dependencies.

### Start a process through SSH without having it exit when SSH connection is lost 
```
$ screen
$ <command>

# ...running...
# detatch "ctrl+a" and "d"
# resume session

$ screen -r
```

Started working from these:
- https://github.com/GoogleCloudPlatform/cpp-docs-samples/blob/master/iot/mqtt-ciotc/mqtt_ciotc.c
- https://github.com/technion/lol_dht22/blob/master/dht22.c

**Note to self:** What was the Dockefile for? D: Just for installing deps, I think?
