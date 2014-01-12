 #!/bin/sh
echo "Content-type: text/html"
echo ""

if [ -d /sys/class/gpio/gpio17/ ] 
then
  state="`cat /sys/class/gpio/gpio17/value`"
  [ "$state" = "1"] && echo "pressed"
  [ "$state" = "0"] && echo "not pressed"
else
  echo "button gpio pin not exported"
fi
