SUMMARY = "U-boot boot scripts for Raspberry Pi"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"
COMPATIBLE_MACHINE = "^rpi$"

DEPENDS = "u-boot-mkimage-native"

BOOT_CMD = "${@bb.utils.contains('DISTRO_FEATURES', 'uota', 'boot.cmd.uota', 'boot.cmd.in', d)}"
SRC_URI = "file://${BOOT_CMD}"

do_compile() {
    sed -e 's/@@KERNEL_IMAGETYPE@@/${KERNEL_IMAGETYPE}/' \
        -e 's/@@KERNEL_BOOTCMD@@/${KERNEL_BOOTCMD}/' \
        "${WORKDIR}/${BOOT_CMD}" > "${WORKDIR}/boot.cmd"
    mkimage -A arm -T script -C none -n "Boot script" -d "${WORKDIR}/boot.cmd" boot.scr
}

inherit deploy

do_deploy() {
    install -d ${DEPLOYDIR}
    install -m 0644 boot.scr ${DEPLOYDIR}
}

addtask do_deploy after do_compile before do_build
