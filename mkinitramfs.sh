#!/usr/bin/fakeroot /bin/sh

echo "fakeing root enviroment"

echo "creating temp dir"                                                                        
mkdir tmpmnt

echo "creating dev and nods"
mkdir tmpmnt/dev
mknod tmpmnt/dev/console c 5 1 
mknod tmpmnt/dev/loop0 b 7 0 

echo "copying files"
cp -r $1/* tmpmnt

cd tmpmnt
find . | cpio -H newc -o > ../initramfs.cpio
cd ..  


echo "xzing initrd"
rm initramfs.cpio.xz
xz -z initramfs.cpio

echo "tmpmnt"
rm -rf tmpmnt
