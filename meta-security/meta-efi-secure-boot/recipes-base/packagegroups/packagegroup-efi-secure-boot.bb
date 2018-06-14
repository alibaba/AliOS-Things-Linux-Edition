DESCRIPTION = "EFI Secure Boot packages for secure-environment."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "\
    file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302 \
"

S = "${WORKDIR}"

ALLOW_EMPTY_${PN} = "1"

pkgs = "\
    grub-efi \
    efitools \
    efibootmgr \
    mokutil \
    seloader \
    shim \
"

RDEPENDS_${PN}_x86 = "${pkgs}"
RDEPENDS_${PN}_x86-64 = "${pkgs}"

kmods = "\
    kernel-module-efivarfs \
    kernel-module-efivars \
"

RRECOMMENDS_${PN}_x86 += "${kmods}"
RRECOMMENDS_${PN}_x86-64 += "${kmods}"

IMAGE_INSTALL_remove += "grub"
