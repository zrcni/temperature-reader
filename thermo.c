#define _XOPEN_SOURCE 500
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <MQTTClient.h>
#include "conditions.h"

int DEBUG = 0; /* Set to 1 to enable debugging */

struct {
  char* address;
  enum { clientid_maxlen = 256, clientid_size };
  char clientid[clientid_size];
  char* deviceid;
  enum { topic_maxlen = 256, topic_size };
  char topic[topic_size];
  int interval; // Interval of the read loop
  char* pin; // WiringPi pin number (RPi)
} opts = {
  .address = "tcp://127.0.0.1:1883",
  .clientid = "rpi",
  .deviceid = "rpi",
  .topic = "home/livingroom/temperature",
  .interval = 0,
  .pin = "7"
};

void Usage() {
  printf("\t--deviceid <your device id>\n");
  printf("\t--pin <e.g. 7>\n");
  printf("\t--interval <seconds>\n");
  printf("\t--debug\n");
}

bool GetOpts(int argc, char** argv) {
  int pos = 1;
  bool calcvalues = false;
  bool calctopic = true;

  if (argc < 2) {
    return false;
  }

  while (pos < argc) {
    if (strcmp(argv[pos], "--deviceid") == 0) {
      if (++pos < argc) {
        if (DEBUG) {
          printf("device id: %s\n", argv[pos]);
        }

        opts.deviceid = argv[pos];
        calcvalues = true;
      } else {
        return false;
      }
    } else if (strcmp(argv[pos], "--topic") == 0) {
      if (++pos < argc) {
        if (DEBUG) {
          printf("topic: %s\n", argv[pos]);
        }

        strcpy((char * restrict)&opts.topic,argv[pos]);
        calctopic=false;
      } else {
        return false;
      }
    } else if (strcmp(argv[pos], "--interval") == 0) {
      if (++pos < argc) {
        if (DEBUG) {
          printf("interval: %s\n", argv[pos]);
        }

        opts.interval = atoi(argv[pos]);
      } else {
        return false;
      }
    } else if (strcmp(argv[pos], "--pin") == 0) {
      if (++pos < argc) {
        if (DEBUG) {
          printf("pin: %s\n", argv[pos]);
        }

        opts.pin = argv[pos];
      } else {
        return false;
      }
    } else if (strcmp(argv[pos], "--debug") == 0) {
      DEBUG = 1;
    }
    pos++;
  }
  if (calctopic) {
    int n = snprintf(opts.topic, sizeof(opts.topic),
        "home/livingroom/temperature",
        opts.deviceid);
    if (n < 0) {
      printf("Encoding error!\n");
      return false;
    }
    if (n > sizeof(opts.topic)) {
      printf("Error, buffer for storing device ID was too small.\n");
      return false;
    }
  }

  if (calcvalues) {
    if (DEBUG) {
      printf("client id: ");
      printf("%s\n", opts.clientid);
    }

    return true; // Caller must free opts.clientid
  }
  return false;
}

// at least once
static const int kQos = 1;
static const unsigned long kTimeout = 10000L;

static const unsigned long kInitialConnectIntervalMillis = 500L;
static const unsigned long kMaxConnectIntervalMillis = 6000L;
static const unsigned long kMaxConnectRetryTimeElapsedMillis = 900000L;
static const float kIntervalMultiplier = 1.5f;

MQTTClient client = {0};
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token = {0};

int Connect() {

  int rc = -1;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

  MQTTClient_create(&client, opts.address, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

  conn_opts.keepAliveInterval = 60;
  conn_opts.cleansession = 1;

  unsigned long retry_interval_ms = kInitialConnectIntervalMillis;
  unsigned long total_retry_time_ms = 0;

  while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
    if (rc == 3) {  // connection refused: server unavailable
      usleep(retry_interval_ms / 1000);
      total_retry_time_ms += retry_interval_ms;
      if (total_retry_time_ms >= kMaxConnectRetryTimeElapsedMillis) {
        printf("Failed to connect, maximum retry time exceeded.");
        return EXIT_FAILURE;
      }
      retry_interval_ms *= kIntervalMultiplier;
      if (retry_interval_ms > kMaxConnectIntervalMillis) {
        retry_interval_ms = kMaxConnectIntervalMillis;
      }
    } else {
      printf("Failed to connect, return code %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  return 0;
}

int Publish(char* payload, int payload_size) {
  int rc = -1;
  pubmsg.payload = payload;
  pubmsg.payloadlen = payload_size;
  pubmsg.qos = kQos;
  pubmsg.retained = 0;
  MQTTClient_publishMessage(client, opts.topic, &pubmsg, &token);

  if (DEBUG) {
    printf("Waiting for up to %lu seconds for publication of %s\n"
          "on topic %s\n",
          (kTimeout/1000), payload, opts.topic, opts.clientid);
  }

  rc = MQTTClient_waitForCompletion(client, token, kTimeout);

  if (DEBUG) {
    printf("Message with delivery token %d delivered\n", token);
  }
  return rc;
}

int main(int argc, char* argv[]) {
  if (!GetOpts(argc, argv)) {
    Usage();
    return 1;
  }

  while (1) {
    char data[32];
    read_sensor(opts.pin, data, 32);
    struct Conditions conditions = get_conditions(data);

    // mock for developing without sensors
    // struct Conditions conditions;
    // conditions.temperature = 22.2;
    // conditions.humidity = 34.2;

    if (DEBUG) {
      printf("Temperature: %.2f\n", conditions.temperature);
      printf("Humidity: %.2f\n", conditions.humidity);
    }

    time_t unix_timestamp = time(NULL);

    char payload[128];
    int written = sprintf(payload, "{\"temperature\":%.2f,\"humidity\":%.2f,\"timestamp\":%d}",
      conditions.temperature, conditions.humidity, unix_timestamp);

    Connect();
    Publish(payload, written);
    MQTTClient_disconnect(client, 5000);

    if (opts.interval == 0) {
      if (DEBUG) {
        printf("Exiting since interval was not provided\n");
      }
      break;
    }

    if (DEBUG) {
      printf("Getting the next reading in %d seconds\n", opts.interval);
    }

    sleep(opts.interval);
  }


  MQTTClient_destroy(&client);

  return 0;
}
