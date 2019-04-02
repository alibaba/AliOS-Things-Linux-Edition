SUMMARY = "ultra light-weight container runtime"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

DEPENDS = "curl util-linux"
DEPENDS_class-native = "curl-native"

SRC_URI = "file://ucontainer-${PV}.tar.gz;subdir=${S} \
           file://ucontainer_network.sh \
		  "

inherit cmake

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/uContainer ${D}${bindir}
	install -m 0755 ${WORKDIR}/ucontainer_network.sh ${D}${bindir}
}

FILES_${PN} = "${bindir}"
