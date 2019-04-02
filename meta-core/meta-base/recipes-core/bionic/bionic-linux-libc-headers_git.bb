SUMMARY = "Processed set of the uapi Linux kernel headers for the bionic C library's use"
LICENSE = "GPLv2"
SECTION = "devel"
PROVIDES = "linux-libc-headers"

require bionic.inc

LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

do_compile () {
    :
}

do_install () {
    install -d ${D}${includedir}
    oe_runmake install-kernel-headers
}