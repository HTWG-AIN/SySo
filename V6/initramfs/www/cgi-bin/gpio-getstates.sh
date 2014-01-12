#!/bin/sh

echo "Content-type: text/html"
echo ""



if [ ! -d /sys/class/gpio/gpio17/ ] 
then
  echo "17" > /sys/class/gpio/export
fi
state="`cat /sys/class/gpio/gpio17/value`"

[ "$state" = "1" ] && echo "button = not pressed"
[ "$state" = "0" ] && echo "button = pressed"

if [ -d /sys/class/gpio/gpio18 ] 
then
  state=`cat /sys/class/gpio/gpio18/value`
  [ "$state" = "1" ] && echo "led = off"
  [ "$state" = "0" ] && echo "led = on"
else
  echo "led gpio port not exported"
fi
