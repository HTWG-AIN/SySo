#!/bin/sh
echo "Content-type: text/html"
echo ""

if [ -d /sys/class/gpio/gpio18 ] 
then
   state=`cat /sys/class/gpio/gpio18/value`
   [ "$state" = "1" ] && echo "off"
   [ "$state" = "0" ] && echo "on"
else
  echo "led gpio port not exported"
fi
