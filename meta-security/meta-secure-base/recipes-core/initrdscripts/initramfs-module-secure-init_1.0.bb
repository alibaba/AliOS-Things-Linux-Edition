SUMMARY = "initramfs-framework module for live booting"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
RDEPENDS_${PN} = "initramfs-framework-base udev-extraconf"

inherit allarch

SRC_URI = "file://secure-init"

S = "${WORKDIR}"

do_install() {
    install -d ${D}/init.d
    install -m 0755 ${WORKDIR}/secure-init ${D}/init.d/81-secure-init
}

FILES_${PN} = "/init.d/81-secure-init"

RDEPENDS_${PN} += "initramfs-framework-base"