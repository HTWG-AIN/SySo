#!/bin/sh
echo "Content-type: text/html"
echo ""

network_info=`ifconfig`
echo "$network_info"
