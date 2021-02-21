# Note that these are not environment variables
CC=gcc # gcc || clang
CCOPTIONS=-std=c11 -Wall -Wextra -pedantic -Wstrict-overflow -fno-strict-aliasing
SSLPATH=-L/openssl
LD_LIBRARY_PATH=-L/usr/local/lib
C_INCLUDE_PATH=-I/usr/local/include

build_thermo: thermo.c
	$(CC) $(CCOPTIONS) $(SSLPATH) $(LD_LIBRARY_PATH) $(C_INCLUDE_PATH) thermo.c conditions.c conditions.h $(BUILDARGS) -lcrypto -lssl -lpaho-mqtt3cs -o ./bin/thermo

build_read_sensor: read_sensor.c
	cc -Wall read_sensor.c -o read_sensor -lwiringPi
