FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:${THISDIR}/files:"
SRC_URI += " \
        file://CVE-2017-16544.patch \
        file://0001-Replace-int-uint-to-avoid-signed-integer-overflow.patch \
        file://CVE-2017-15873.patch \
        file://ntpd.cfg \
        file://busybox-ntpd \
        file://ntp.conf \
        "

SRC_URI_append_libc-bionic = " file://bionic.cfg "
SRC_URI_append_libc-bionic = " file://0001-Add-supports-for-bionic.patch "

PACKAGES =+ "${PN}-ntpd"

FILES_${PN}-ntpd = "${sysconfdir}/init.d/busybox-ntpd ${sysconfdir}/ntp.conf"

INITSCRIPT_PACKAGES += "${PN}-ntpd"
INITSCRIPT_NAME_${PN}-ntpd = "busybox-ntpd"
INITSCRIPT_PARAMS_${PN}-ntpd = "start 99 2 3 4 5 . stop 01 0 1 6 ."

do_install_append () {
    if grep "CONFIG_NTPD=y" ${B}/.config; then
        install -m 0755 ${WORKDIR}/busybox-ntpd ${D}${sysconfdir}/init.d/
        install -m 0644 ${WORKDIR}/ntp.conf ${D}${sysconfdir}/
    fi
}

