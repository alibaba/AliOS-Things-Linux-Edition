# Modify default system configuration  
FILESEXTRAPATHS_prepend := "${THISDIR}/systemd:"
SRC_URI_append += "file://50-wired.network"

do_install_append() {
    # Config systemd DHCP on wired ethernet port
    cp ${WORKDIR}/50-wired.network ${D}/lib/systemd/network
	
	# Only forward err message to syslog
	sed -i -e 's/.*MaxLevelSyslog.*/MaxLevelSyslog=err/' ${D}${sysconfdir}/systemd/journald.conf
}
