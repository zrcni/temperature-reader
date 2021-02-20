CC=gcc # gcc || clang
CCOPTIONS=-std=c11 -Wall -Wextra -pedantic -Wstrict-overflow -fno-strict-aliasing
SSLPATH=-L/openssl
LD_LIBRARY_PATH=/usr/local/lib

build_thermo: thermo.c
	$(CC) $(CCOPTIONS) $(SSLPATH) thermo.c conditions.c conditions.h $(BUILDARGS)  -ljwt -lcrypto -lssl -lpaho-mqtt3cs -o ./bin/thermo

build_read_sensor: read_sensor.c
	cc -Wall read_sensor.c -o read_sensor -lwiringPi