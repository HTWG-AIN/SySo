#!/bin/sh

#tmp vollschreiben
mkdir /tmp/blub
mkdir /tmp/bla

loadkmap < /etc/keymaps/keymap_de_DE &

udhcpc -i eth0 -s /etc/udhcpc/simple.script
httpd -h /www/

sysinfo-printer
