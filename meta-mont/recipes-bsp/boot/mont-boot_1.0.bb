DESCRIPTION = "Montage boot code"
MNT_BOOT_IMAGE = "boot-${MACHINE}-${PV}-${PR}.img"

LICENSE = "CLOSED"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://boot"

S = "${WORKDIR}/boot"

COMPATIBLE_MACHINE = "mont-panther"

EXTRA_OEMAKE = "LE=1"
PARALLEL_MAKE = "-j 1"
DEPENDS += "util-linux-native lzop-native"

ALLOW_EMPTY_${PN} = "1"

do_deploy () {
    install -d ${DEPLOY_DIR_IMAGE}
    install -m 644 ${S}/boot.img ${DEPLOY_DIR_IMAGE}/${MNT_BOOT_IMAGE}
}

addtask deploy after do_compile
