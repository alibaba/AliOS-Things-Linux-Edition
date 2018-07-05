DESCRIPTION = "Extra files for via-image-x11 image"
LICENSE = "CLOSED"

SRC_URI = "file://viabsp_ver"

do_install () {
    install -d ${D}/etc/
	install -m 644 ${WORKDIR}/viabsp_ver ${D}/etc/
    install -d ${D}/etc/modprobe.d
    echo "options ath9k btcoex_enable=1" > ${D}/etc/modprobe.d/ath9k_btcoex.conf
}
