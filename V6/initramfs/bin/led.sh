#!/bin/sh
RUN_FLAG=1
# pin freigeben durch signal handler
trap RUN_FLAG=0 EXIT SIGINT SIGHUP SIGTERM SIGQUIT
#pin besschlagnahmen.
echo "18" >  /sys/class/gpio/export
# ausgang setzten
echo "out" > /sys/class/gpio/gpio18/direction 
echo 1 >   /sys/class/gpio/gpio18/value


while [ $RUN_FLAG -eq 1 ]
do

  # led einschalten
  echo 0 >   /sys/class/gpio/gpio18/value
  sleep 1
  # led ausschalten.
  echo 1 >   /sys/class/gpio/gpio18/value
  sleep 1



done

echo "18" >/sys/class/gpio/unexport




