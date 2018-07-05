IMAGE_INSTALL_append = " \
    ${@bb.utils.contains("DISTRO_FEATURES", "tpm", \
                         "packagegroup-tpm", "", d)} \
    ${@bb.utils.contains("DISTRO_FEATURES", "tpm2", \
                         "packagegroup-tpm2", "", d)} \
    ${@bb.utils.contains("DISTRO_FEATURES", "ima", \
                         "packagegroup-ima", "", d)} \
    ${@bb.utils.contains("DISTRO_FEATURES", "luks", \
                         "packagegroup-luks", "", d)} \
    ${@bb.utils.contains("DISTRO_FEATURES", "ids", \
                         "packagegroup-ids", "", d)} \
"

INITRAMFS_IMAGE = "core-image-minimal-initramfs"