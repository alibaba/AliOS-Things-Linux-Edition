SUMMARY = "A wireless mesh network stack implementation of AliOS Things Linux Edition"
DESCRIPTION = "uMesh, provides the capabilities of organizing network by themselves and achieving \
local interconnection among devices. uMesh is characterized as self-organized, \
self-healing and multi-hop, and It's suitable for scenarios that need large-scale deployment."
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

SRC_URI = "file://umesh-${PV}.tar.gz"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile() {
    make ARCH=${TARGET_ARCH}
}

do_install() {
    install -d ${D}${libdir}
    install -d ${D}${bindir}
    install -d ${D}${includedir}

    install -m 0664 ${B}/libmesh.a ${D}${libdir}

    install -m 0644 ${S}/test/include/umesh.h ${D}${includedir}
    install -m 0644 ${S}/test/include/umesh_config.h ${D}${includedir}
    install -m 0644 ${S}/test/include/umesh_types.h ${D}${includedir}
    install -d ${D}${includedir}/aos
    install -m 0644 ${S}/test/include/aos/list.h ${D}${includedir}/aos/

    # mesh_test is a test program for uMesh.
    install -m 0755 ${B}/mesh_test ${D}${bindir}
}

