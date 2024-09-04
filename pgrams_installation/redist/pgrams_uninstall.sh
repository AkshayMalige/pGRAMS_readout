#!/bin/bash

echo "Uninstalling pgrams driver..."

if test -d /lib/modules/`uname -r`/kernel/drivers ; then
	MODDIR=/lib/modules/`uname -r`/kernel/drivers/misc
else
	MODDIR=/lib/modules/`uname -r`/misc
fi

kp=kp_pgrams_module
module_path=$MODDIR"/pgrams.ko"
usb_path=$MODDIR"/pgrams_usb.ko"
sriov_path=$MODDIR"/pgrams_sriov.ko"

if lsmod | grep $kp &> /dev/null ; then
    echo "Removing $kp..."
    sudo rmmod $kp
fi

echo "Removing pgrams driver..."
sudo modprobe -r pgrams
sudo rm -f /etc/.windriver.rc
sudo rm -f $HOME/.windriver.rc

echo "Removing shared libraries..."
sudo rm -f /usr/lib/libwdapi1630.so
sudo rm -f /usr/lib/libwdapi1630_32.so
sudo rm -f /usr/lib64/libwdapi1630.so

echo "Removing pgrams modules..."

sudo rm -f $sriov_path
sudo rm -f $usb_path
sudo rm -f $module_path

PATH="`dirname \"$0\"`"
PATH="`( cd \"$PATH\" && pwd )`"

cd $PATH
cd ../..

echo ""
echo "Done!"
echo "You can now remove the whole `pwd` folder!"

