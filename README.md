**Currently doesn't go through Google PubSub, but instead publishes to an MQTT broker running in the local network. Leaving the documentation untouched for now.**

**Note to self:** What was the Dockefile for? D: Just installing deps?

## Temperature/humidity readings 

**Main** program reads condition measurements and publishes the data to Google PubSub through the MQTT Bridge

- **Reader**: C program that reads measurements from a DHT22 sensor. Writes temperature and humidity to stdout
- **Main**: C program that reads measurements through **Reader** and publishes the data to Google PubSub

Started working from these:
- https://github.com/GoogleCloudPlatform/cpp-docs-samples/blob/master/iot/mqtt-ciotc/mqtt_ciotc.c
- https://github.com/technion/lol_dht22/blob/master/dht22.c

```
./bin/thermo --deviceid <device-id> --region <region-name> --registryid <registry-name> --projectid <project-id> --pin <wiringpi-pin-number> --interval <seconds>
```

### Currently works like this:
1. Parse options from args
2. Set up authentication for MQTT client
3. Create MQTT client
4. Repeat every X seconds
    - read data
    - connect
    - publish data
    - disconnect

MQTT client connection requires [certs](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge): roots.pem, rsa_public.pem, rsa_private.pem

Currently compiling the C programs on the Raspberry Pi, because I haven't figured out how to cross-compile dependencies.

### Start a process through SSH without having it exit when SSH connection is lost 
```
$ screen
$ <command>

# ...running...
# detatch "ctrl+a" and "d"
# resume session

$ screen -r
```