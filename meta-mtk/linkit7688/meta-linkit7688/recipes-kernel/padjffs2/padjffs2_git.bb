DESCRIPTION = "Openwrt tool for pathcing an image with DTB file."
SECTION = "Openwrt tools."
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "git://github.com/openwrt/openwrt.git;protocol=git"

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git/tools/padjffs2/src"

BBCLASSEXTEND="native nativesdk"

FILES_${PN}_class-native="${D}/${bindir}/*"

do_compile_class-native () {
    ${CC} padjffs2.c ${LDFLAGS} -o padjffs2
}

do_install(){
    install -d ${D}/${bindir}/
    install -m 0755 ${S}/padjffs2 ${D}/${bindir}/ 
}
