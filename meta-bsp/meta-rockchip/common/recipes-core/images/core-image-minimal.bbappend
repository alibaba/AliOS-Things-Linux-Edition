IMAGE_INSTALL_append += " kernel-modules"

# Strip all free spaces in rootfs
IMAGE_ROOTFS_EXTRA_SPACE = "0"
IMAGE_OVERHEAD_FACTOR = "1"

# For ap6255 wifi
IMAGE_INSTALL_append += " linux-firmware-bcm43455"

# For wifi test
IMAGE_INSTALL_append += " iw wpa-supplicant"

# For codec test
IMAGE_INSTALL_append += " alsa-utils"
