#!/bin/sh
echo "Content-type: text/html"
echo ""
os=`uname -o`
platform=`uname -i`
machine=`uname -m`
kernel_version=`uname -v`
kernel_release=`uname -r`
net_node_name=`uname -n`
kernel_name=`uname -s`

echo "$kernel_name $kernel_release $kernel_version $machine $platform $os"
