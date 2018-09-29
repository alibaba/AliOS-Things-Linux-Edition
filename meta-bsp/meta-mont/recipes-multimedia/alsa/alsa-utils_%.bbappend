FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

COMPATIBLE_MACHINE_mont-panther = "mont-panther"

SRC_URI += "file://asound.conf \
           "
FILES_${PN} += "/etc/asound.conf"

do_install_append() {
	install -d ${D}/etc
	install -m 0644 ${WORKDIR}/asound.conf ${D}/etc/asound.conf
}
