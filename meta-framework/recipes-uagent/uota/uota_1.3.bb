SUMMARY = "The OTA framework of AliOS Things Linux Edition"
DESCRIPTION = "An over-the-air update is the wireless delivery of new software or data to smart \
devices, especially IoT devices. Wireless carriers and OEMs typically use over-the-air (OTA) \
updates to deploy the new operating systems and the software app to these devices."
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

SRC_URI = "file://uota-1.3-20190314.tgz"

TARGET_CC_ARCH += "${LDFLAGS}"

DEPENDS += "mbedtls"

do_compile() {
    make ARCH=${TARGET_ARCH}
}

do_install() {
    install -d ${D}${libdir}
    install -d ${D}${bindir}
    install -d ${D}${includedir}

    # The OTA_APP is an example that use the OTA library.
    install -m 0755 ${B}/OTA_APP ${D}${bindir}
    install -m 0644 ${B}/libOTA.a  ${D}${libdir}
    install -m 0644 ${B}/libLK.a  ${D}${libdir}
    install -m 0644 ${B}/inc/ota_service.h ${D}${includedir}
}

