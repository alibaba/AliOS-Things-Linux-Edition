DESCRIPTION = "The packages used for luks in initramfs."

require packagegroup-luks.inc

RDEPENDS_${PN} += "\
    cryptfs-tpm2-initramfs \
    ${@bb.utils.contains('DISTRO_FEATURES', 'tpm2', 'packagegroup-tpm2-initramfs', '', d)} \
"
