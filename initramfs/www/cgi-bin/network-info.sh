#!/bin/sh
echo "Content-type: text/html"
echo ""

network_info=`ip addr`
echo "$network_info"
