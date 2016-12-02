#!/bin/bash


sudo rsync -a /lib $HOME/lib_backup
sudo rsync -a /boot $HOME/boot_backup

cd lib

sudo cp -rd * /lib/

cd ..

KERNEL=kernel7

sudo cp boot/dts/*.dtb /boot/

sudo cp boot/dts/overlays/*.dtb* /boot/overlays

sudo cp boot/dts/overlays/README /boot/overlays

sudo ./mkknlimg boot/zImage /boot/$KERNEL.img
