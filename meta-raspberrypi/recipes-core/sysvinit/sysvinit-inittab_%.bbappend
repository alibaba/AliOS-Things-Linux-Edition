COMPATIBLE_MACHINE = "raspberrypi3-64"

do_install() {
    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/inittab ${D}${sysconfdir}/inittab
    install -d ${D}${base_bindir}
    install -m 0755 ${WORKDIR}/start_getty ${D}${base_bindir}/start_getty

    set -x
    tmp="${SERIAL_CONSOLES}"
    if [ ${ENABLE_UART} != "" ]; then
        for i in $tmp
        do
            j=`echo ${i} | sed s/\;/\ /g`
            l=`echo ${i} | sed -e 's/tty//' -e 's/^.*;//' -e 's/;.*//'`
            label=`echo $l | sed 's/.*\(....\)/\1/'`
            echo "$label:12345:respawn:${base_bindir}/start_getty ${j} vt102" >> ${D}${sysconfdir}/inittab
        done
    fi

    if [ "${USE_VT}" = "1" ]; then
        cat <<EOF >>${D}${sysconfdir}/inittab
# ${base_sbindir}/getty invocations for the runlevels.
#
# The "id" field MUST be the same as the last
# characters of the device (after "tty").
#
# Format:
#  <id>:<runlevels>:<action>:<process>
#

EOF

        for n in ${SYSVINIT_ENABLED_GETTYS}
        do
            echo "$n:12345:respawn:${base_sbindir}/getty 38400 tty$n" >> ${D}${sysconfdir}/inittab
        done
        echo "" >> ${D}${sysconfdir}/inittab
    fi
}

