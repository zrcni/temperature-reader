#!/bin/bash

# 5 minutes
seconds=300

rxp='^[0-9]+$'
if [[ $1 =~ $rxp ]]; then
  seconds=$1
fi

printf "Getting a temperature reading every %s seconds\n" $seconds

while true; do
  ./bin/thermo \
    --pin 7 \
    --topic home/livingroom/temperature \
    --address tcp://127.0.0.1:1884 \
    --deviceid rpi \
    --clientid rpi

  if [ $? -eq 0 ]; then
    printf "Published a reading successfully\n"
  fi

  printf "Next reading in %s seconds\n" $seconds

  sleep $seconds
done
