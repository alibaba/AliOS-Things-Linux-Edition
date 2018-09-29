DESCRIPTION = "AliOS Things Linux Edition comboapp"
SUMMARY = "AliOS Things Linux Edition comboapp"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

#CFLAGS_prepend = " -g "

#DEPENDS += "bluez5"
#DEPENDS := "${@bb.utils.contains('DISTRO_FEATURES','bluez5','bluez5','bluez4',d)}"

SRC_URI = "file://linkkit-0.6.tar.gz \
           file://combo.init \
           file://linkkit.kv"

# only support sysvinit for now.
inherit update-rc.d

INITSCRIPT_NAME = "combo"
INITSCRIPT_PARAMS = "defaults 87"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile() {
    make ARCH=${TARGET_ARCH} -f Makefile.comboapp wifi_module=rtk ble_module=rtk
}

do_install() {
    # create linkkit dir
    install -d ${D}/${bindir}/
    install -d ${D}/${datadir}/linkkit/
    install -d ${D}/${INIT_D_DIR}
    install -m 0644 ${WORKDIR}/linkkit.kv ${D}/${datadir}/linkkit/linkkit.kv
    install -m 0755 ${WORKDIR}/combo.init ${D}/${INIT_D_DIR}/combo
    install -m 0755 ${B}/comboapp ${D}/${bindir}/comboapp
}
