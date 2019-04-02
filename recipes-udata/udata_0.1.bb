DESCRIPTION = "udata"
SUMMARY = "udata"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

SRC_URI = "file://udata-0.1.tgz"

EXTRA_OEMAKE = "'CC=${CC}' 'AR=${AR}' 'CFLAGS=${CFLAGS}' 'BUILDDIR=${S}'"
TARGET_CC_ARCH += "${LDFLAGS}"

DEPENDS += "mbmaster"


do_compile() {
    make ARCH=${TARGET_ARCH}
}

sysroot_stage_all_append() {
    mkdir -p ${SYSROOT_DESTDIR}${includedir}
    sysroot_stage_dir ${S}/iotx-sdk-c_clone/output/release/include ${SYSROOT_DESTDIR}${includedir}
    mkdir -p ${SYSROOT_DESTDIR}${libdir}
    sysroot_stage_dir ${S}/iotx-sdk-c_clone/output/release/lib ${SYSROOT_DESTDIR}${libdir}
}

do_install() {
    install -d ${D}${libdir}
    install -d ${D}${bindir}

    install -m 0755 ${B}/libudata.a ${D}${libdir}
    install -m 0755 ${B}/udata_app ${D}${bindir}
}

FILES_${PN} = "${bindir} ${libdir}"
