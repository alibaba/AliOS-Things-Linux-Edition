
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
	file://50-wired.network \
	file://enabled-usb-wakeup.conf \
	"

do_install_append() {
    install -m 644 ${WORKDIR}/50-wired.network ${D}/lib/systemd/network/
    install -m 644 ${WORKDIR}/enabled-usb-wakeup.conf ${D}/etc/tmpfiles.d/
	
	# Only forward err message to syslog
	sed -i -e 's/.*MaxLevelSyslog.*/MaxLevelSyslog=err/' ${D}${sysconfdir}/systemd/journald.conf
}
