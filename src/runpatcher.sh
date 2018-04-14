#!/bin/sh

# name of front-end executable
EXEC=/root/ui_test1

is_mod_hosts() {
	# check for our old backup
	[[ -e ${MOUNTDIR}/etc/hosts.old ]] && echo "1" || echo "0"
}

is_hidden_setting() {
	# check for our patched setting
	grep '<a key="uploadlogs" value="0"/>' \
			 ${MOUNTDIR}/nestlabs/etc/client.conf \
			 2>/dev/null 1>/dev/null
	echo ${?}
}

mod_hosts() {
	# if we do not have a backup of hosts file
	if ! [ -e ${MOUNTDIR}/etc/hosts.old ]; then
		# make one
		cp ${MOUNTDIR}/etc/hosts ${MOUNTDIR}/etc/hosts.old
		# then redirect log servers to the loopback...
		for i in $(seq -w 0 99); do
			echo "log-rts${i}-iad01.devices.nest.com	127.0.0.1" >> ${MOUNTDIR}/etc/hosts
		done
	fi
}

undo_hosts() {
	# restore hosts file form backup
	if [ -e ${MOUNTDIR}/etc/hosts.old ]; then
		mv ${MOUNTDIR}/etc/hosts.old ${MOUNTDIR}/etc/hosts
 	fi
}

clear_logs() {
	# remove all logs
	rm -rf ${MOUNTDIR}/var/log/
}

hidden_setting() {
	# sed magic
	sed -i \
		's#<a key="uploadlogs" value="1"/>#<a key="uploadlogs" value="0"/>#g' \
		${MOUNTDIR}/nestlabs/etc/client.config
}

undo_setting() {
	# more sed magic
	sed -i \
		's#<a key="uploadlogs" value="0"/>#<a key="uploadlogs" value="1"/>#g' \
		${MOUNTDIR}/nestlabs/etc/client.config
}

# for live demonstration purposes, stop monit and
# kill nlclient
/etc/init.d/monit stop
sleep 1
pkill nlclient && sleep 2 || exit 1

# run front-end and obtain return value
${EXEC} $(is_mod_hosts) $(is_hidden_setting) 0
retval=${?}

# some awk magic to extract the bits
[[ $(awk "BEGIN {print and(${retval}, 4)}") == 4 ]] && mod_hosts || undo_hosts
[[ $(awk "BEGIN {print and(${retval}, 2)}") == 2 ]] && hidden_setting || undo_setting
[[ $(awk "BEGIN {print and(${retval}, 1)}") == 1 ]] && clear_logs

/root/done_patching

# for demo purposes, restart services
sleep 100
/etc/init.d/nestlabs start
/etc/init.d/monit start

# and we are done
