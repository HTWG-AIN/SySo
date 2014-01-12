#!/bin/sh
echo "Content-type: text/html"
echo ""

if [ -d /sys/class/gpio/gpio18/ ] 
then
  echo "out" > /sys/class/gpio/gpio18/direction
  echo "1" > /sys/class/gpio/gpio18/value
else
  echo "18" > /sys/class/gpio/export
  echo "out" > /sys/class/gpio/gpio18/direction
  echo "1" > /sys/class/gpio/gpio18/value
  echo "18" > /sys/class/gpio/unexport
fi
echo "port switched off";





