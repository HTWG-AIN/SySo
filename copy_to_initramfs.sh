#!/bin/sh
#[ -z $1 ] && echo "usage: $0 <dir_to_be_copied> <initramfs-directory>" && exi
#[ -z $2 ] && echo "usage: $0 <dir_to_be_copied> <initramfs-directory>" && exi

source=~/git-arm/initramfs
target=~/buildroot/output/target
echo "Calling make..."
cd ~/git-arm
make
echo "copying files from '$source' to '$target'"
cp -r $source/* $target
