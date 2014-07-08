#!/bin/sh
cd $(dirname $(readlink -f $0))
PATH="${PATH}:.:/home/henrik_n/asic_cmd"

case "$reason" in
PREINIT)
	lcd-message "Connecting.."
	;;
RELEASE)
	lcd-message "Disconnected"
	;;
BOUND)
	lcd-message "${new_ip_address}"
	;;
RENEW)
	lcd-message "${new_ip_address}"
	;;
esac
