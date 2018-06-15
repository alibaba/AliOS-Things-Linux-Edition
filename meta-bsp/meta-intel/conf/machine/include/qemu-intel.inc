# Similar to meta/conf/machine/include/qemu.inc but influences the
# distro content (in particular the kernel) less than qemu.inc.

# Ensure that qemu gets built when building images.
EXTRA_IMAGEDEPENDS += "qemu-native qemu-helper-native"

# Build ovmf firmware for uefi support in qemu.
EXTRA_IMAGEDEPENDS += "ovmf"

# runqemu uses virtio, so we have to enable the corresponding
# kernel drivers.
KERNEL_FEATURES_INTEL_COMMON += "cfg/virtio.scc"

# Other useful extensions which are not enabled by default:
# rng-tools to take advantage of qemu's virtio-rng-pci device.
# MACHINE_EXTRA_RRECOMMENDS += "rng-tools"

# Creates the necessary .qemuboot.conf file for each image.
IMAGE_CLASSES += "qemuboot"
