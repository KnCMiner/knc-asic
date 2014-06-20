#!/bin/sh -xe
mount /dev/mmcblk0p1 /boot
if [ ! -f /boot/uEnv.txt.orig ]; then
	cp -p /boot/uEnv.txt /boot/uEnv.txt.orig
fi
echo "optargs=capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN" >>/boot/uEnv.txt
umount /boot
cp BB-* /lib/firmware/
ln -sf $PWD/../rc.local /etc/
sudo ln -s $PWD/../dhclient-lcd.sh /etc/dhcp/dhclient-enter-hooks.d/
#cp rc-local.service /etc/systemd/system/
#systemctl enable rc-local
