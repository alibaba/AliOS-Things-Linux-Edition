DESCRIPTION = "uota"
SUMMARY = "uota is an unified download APP"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

SRC_URI = "file://uota-0.3-20180910.tgz \
           file://uota.init \
           file://uota.kv"

TARGET_CC_ARCH += "${LDFLAGS}"

# only support sysvinit for now.
inherit update-rc.d

do_compile() {
    make ARCH=${TARGET_ARCH}
}

do_install() {
    install -d ${D}/${bindir}/
    install -d ${D}/${datadir}/uota/
    install -d ${D}/${INIT_D_DIR}
    install -m 0644 ${WORKDIR}/uota.kv ${D}/${datadir}/uota/uota.kv
    install -m 0755 ${WORKDIR}/uota.init ${D}/${INIT_D_DIR}/uota
    install -m 0755 ${B}/OTA_APP ${D}/${bindir}/OTA_APP
}

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "uota"
INITSCRIPT_PARAMS = "start 99 2 3 4 5 . stop 01 0 1 6 ."
