#!/bin/sh
echo "Content-type: text/html"
echo ""

if [ ! -d /sys/class/gpio/gpio18/ ] 
then
  echo "18" > /sys/class/gpio/export
  echo "out" > /sys/class/gpio/gpio18/direction
fi


state=`cat  /sys/class/gpio/gpio18/value`

if [ "$state" = "1" ]
then
  echo "0" > /sys/class/gpio/gpio18/value
else
  echo "1" > /sys/class/gpio/gpio18/value
fi




