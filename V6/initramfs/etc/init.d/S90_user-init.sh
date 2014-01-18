#!/bin/sh

# Disable kernel message to print to console
echo 0 > /proc/sys/kernel/printk

echo "

       ____.       __        ___.           
      |    |____  |  | ____ _\_ |__   ______
      |    \__  \ |  |/ /  |  \ __ \ /  ___/
  /\__|    |/ __ \|    <|  |  / \_\ \\___ \ 
  \________(____  /__|_ \____/|___  /____  >
                \/     \/         \/     \/ 
    _________                           .__  .__                     
   /   _____/__ ________   ___________  |  | |__| ____  __ _____  ___
   \_____  \|  |  \____ \_/ __ \_  __ \ |  | |  |/    \|  |  \  \/  /
   /        \  |  /  |_> >  ___/|  | \/ |  |_|  |   |  \  |  />    < 
  /_______  /____/|   __/ \___  >__|    |____/__|___|  /____//__/\_ \\
          \/      |__|        \/                     \/            \/

"

udhcpc -i eth0 -s /etc/udhcpc/simple.script &>/var/log/udhcpc

telnetd &>/var/log/telnet

httpd -h /www/ &>/var/log/httpd

depmod -A
