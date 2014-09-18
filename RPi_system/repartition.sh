#!/bin/sh

### BEGIN INIT INFO
# Provides:          repartition
# Required-Start:
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Re-partitions SD card at first boot
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
NAME=repartition
DESC="SD card repartition check"

FLAGFILE1="/etc/repartition-step1"
FLAGFILE2="/etc/repartition-step1"
TABLEFILE="/etc/table.sfdisk"
DISK="/dev/mmcblk0"
PARTITION="/dev/mmcblk0p2"

do_check() {

    if [ -f $FLAGFILE1 ]; then
        cat $TABLEFILE | sfdisk $DISK
        touch $FLAGFILE2
        rm -f $FLAGFILE1
        reboot
    elif [ -f $FLAGFILE2 ]; then
        resize2fs $PARTITION
        rm -f $FLAGFILE2
    fi

}

case "$1" in
  start)
        echo -n "Starting $DESC: "
        do_check
        echo "$NAME."
        ;;
  stop)
        echo -n "Stopping $DESC: "
        echo "$NAME."
        ;;
  restart|force-reload)
        echo -n "Restarting $DESC: "
        echo "$NAME."
        ;;
  *)
        N=/etc/init.d/$NAME
        echo "Usage: $N {start|stop|restart|force-reload}" >&2
        exit 1
        ;;
esac

exit 0
