#!/bin/sh
[ ! -d /sys/class/gpio/gpio17/ ] && echo "17" > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio17/direction

