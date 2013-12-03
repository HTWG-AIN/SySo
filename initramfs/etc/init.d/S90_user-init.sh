#!/bin/sh

#tmp vollschreiben
mkdir /tmp/blub
mkdir /tmp/bla

#Gerätedateien sollen automatisch angelegt werden.
echo "/sbin/mdev" > /proc/sys/kernel/hotplug

loadkmap < /etc/keymaps/keymap_de_DE &

udhcpc -i eth0 -s /etc/udhcpc/simple.script
httpd -h /www/

sysinfo-printer
