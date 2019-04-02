DESCRIPTION = "Openwrt backports tool for handling kconfig data."
SECTION = "Openwrt tools."
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "file://backports-2017-11-01.tar.xz"

# Modify these as desired
#PV = "1.0+git${SRCPV}"
#SRCREV = "${AUTOREV}"

S = "${WORKDIR}/backports-2017-11-01/kconf"

BBCLASSEXTEND="native nativesdk"

FILES_${PN}_class-native="${D}/${bindir}/*"

do_compile_class-native () {
    make mconf
    make conf
}

do_install(){
    install -d ${D}/${bindir}/
    install -m 0755 ${S}/mconf ${D}/${bindir}/ 
    install -m 0755 ${S}/conf ${D}/${bindir}/ 
}
