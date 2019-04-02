SUMMARY = "The BLE/WiFi combo framework of AliOS Things Linux Edition"
DESCRIPTION = "The combo framework is used for BLE/WiFi provisioning. \
               Please add vendor support for BLE or WiFi HAL when necessary."
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

DEPENDS += " mbedtls"

SRC_URI = "file://combo-2.0.5.tgz"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile() {
    make ARCH=${TARGET_ARCH}
}

do_distclean() {
    make ARCH=${TARGET_ARCH} clean
}

do_install() {
    install -d ${D}${libdir}
    install -d ${D}${includedir}

    install -m 0644 ${B}/libcombo.a  ${D}${libdir}
    install -m 0644 ${B}/breeze/api/*.h ${D}${includedir}
    install -m 0644 ${B}/netmgr/include/netmgr.h ${D}${includedir}
    install -m 0644 ${B}/hal/wifi/wifi.h ${D}${includedir}
    install -m 0644 ${B}/hal/kv/kv.h ${D}${includedir}
}

