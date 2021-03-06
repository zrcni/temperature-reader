#!/bin/bash

seconds=300

rxp='^[0-9]+$'
if [[ $1 =~ $rxp ]]; then
  seconds=$1
fi

printf "Getting a temperature reading every %s seconds\n" $seconds

while true; do
  ./bin/thermo --deviceid rpi --pin 7
  sleep $seconds
  printf "Next reading in %s seconds\n" $seconds
done
