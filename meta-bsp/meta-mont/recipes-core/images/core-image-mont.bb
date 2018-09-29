SUMMARY = "Montage images"

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
#boot
IMAGE_INSTALL += "mont-boot"
#wifi
IMAGE_INSTALL += "wpa-supplicant iw"
#audio
IMAGE_INSTALL += "alsa-utils"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image

IMAGE_ROOTFS_SIZE ?= "8192"
