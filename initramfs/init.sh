#!/bin/sh

#Create all the symlinks to /bin/busybox
#busybox --install -s
for i in `busybox --list`; do /bin/busybox ln -fs /bin/busybox /bin/$i; done

mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t tmpfs tmpfs /tmp

mdev -s

#tmp vollschreiben
mkdir /tmp/blub
mkdir /tmp/bla

# Mounting pseudo terminals for telnet and ssh!
mkdir /dev/pts
mount -t devpts devpts /dev/pts

loadkmap < /etc/keymaps/keymap_de_DE

udhcpc -i eth0 -s /etc/udhcpc/simple.script

httpd -h /www/

exec init
sysinfo-printer

#exec sh -c 'exec sh </dev/tty1 >/dev/tty1 2>&1'
#exec sh
