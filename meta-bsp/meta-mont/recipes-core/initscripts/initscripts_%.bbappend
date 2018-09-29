FILESEXTRAPATHS_prepend := "${THISDIR}/initscripts:"

COMPATIBLE_MACHINE_mont-panther = "mont-panther"

SRC_URI += "file://dw \
            file://ew \
           "

FILES_${PN} += "${base_sbindir}/dw"
FILES_${PN} += "${base_sbindir}/ew"

do_install_append () {
    install -d ${D}${base_sbindir}
	install -m 0755 ${WORKDIR}/dw ${D}${base_sbindir}/dw
	install -m 0755 ${WORKDIR}/ew ${D}${base_sbindir}/ew
}
