DESCRIPTION = "Small image capable of booting a device. The kernel includes \
the Minimal RAM-based Initial Root Filesystem (initramfs), which finds the \
first 'init' program more efficiently."
LICENSE = "MIT"

ROOTFS_BOOTSTRAP_INSTALL_append += "\
    ${@bb.utils.contains("DISTRO_FEATURES", "tpm2", \
                         "packagegroup-tpm2-initramfs", "", d)} \
    ${@bb.utils.contains("DISTRO_FEATURES", "ima", \
                         "packagegroup-ima-initramfs", "", d)} \
    ${@bb.utils.contains("DISTRO_FEATURES", "luks", \
                         "packagegroup-luks-initramfs", "", d)} \
"

PACKAGE_INSTALL = "\
    initrdscripts-secure-core \
    ${VIRTUAL-RUNTIME_base-utils} \
    base-passwd \
    ${ROOTFS_BOOTSTRAP_INSTALL} \
"

# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = ""

export IMAGE_BASENAME = "secure-core-image-initramfs"
IMAGE_LINGUAS = ""

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"

inherit core-image

IMAGE_ROOTFS_SIZE = "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "0"

INITRAMFS_MAXSIZE = "512000"

BAD_RECOMMENDATIONS += "busybox-syslog"
