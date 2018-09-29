DESCRIPTION = "AliOS Things Linux Edition uMeshapp"
SUMMARY = "AliOS Things Linux Edition uMeshapp"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

SRC_URI = "file://umesh-0.1.tar.gz"

EXTRA_OEMAKE = "'CC=${CC}' 'AR=${AR}' 'CFLAGS=${CFLAGS}' 'BUILDDIR=${S}'"
TARGET_CC_ARCH += "${LDFLAGS}"

do_compile() {
    make ARCH=${TARGET_ARCH}
}

do_install() {
    install -d ${D}${libdir}
    install -d ${D}${bindir}

    install -m 0755 ${B}/libmesh.a ${D}${libdir}
    install -m 0755 ${B}/mesh_test ${D}${bindir}
}

FILES_${PN} = "${bindir} ${libdir}"

