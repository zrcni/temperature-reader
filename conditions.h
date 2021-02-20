#include <stdlib.h>

struct Conditions {
  float temperature;
  float humidity;
};

int read_sensor(char* pin, char *dest, size_t size);

struct Conditions get_conditions(char *data);
