#
# BSP variables and settings specific to the meta-intel layer.
# You must include the meta-intel layer in your bblayers.conf
# to use them.
#

#
# XSERVER subcomponents, used to build the XSERVER variable
#

# Soft set linux-yocto as preferred kernel like x86-base.inc
PREFERRED_PROVIDER_virtual/kernel ?= "linux-intel"
PREFERRED_PROVIDER_virtual/kernel_poky-tiny ?= "linux-intel"
PREFERRED_PROVIDER_virtual/kernel_linuxstdbase ?= "linux-yocto"
PREFERRED_VERSION_linux-intel ?= "4.9%"

# for Xserver older than 1.13.0
OLD_XSERVER_X86_EXT = "xserver-xorg-extension-dri \
           xserver-xorg-extension-dri2 \
           xserver-xorg-extension-extmod \
           xserver-xorg-extension-dbe \
           "

XSERVER_X86_MATROX_MGA = "xf86-video-mga \
           "

XSERVER_X86_ASPEED_AST = "xf86-video-ast \
           "

# include the user space intel microcode loading support in the generated images.
MACHINE_ESSENTIAL_EXTRA_RDEPENDS_append = "${@bb.utils.contains('MACHINE_FEATURES', 'intel-ucode', ' intel-microcode', '', d)} thermald"

# recommended extra packages common to all intel machines
MACHINE_EXTRA_RRECOMMENDS_append = " kernel-modules linux-firmware"

# for the early boot time kernel microcode loading support,
# merge the microcode data in the final initrd image.
INITRD_LIVE_prepend = "${@bb.utils.contains('MACHINE_FEATURES', 'intel-ucode', '${DEPLOY_DIR_IMAGE}/microcode.cpio ', '', d)}"

DISK_SIGNATURE_UUID ??= "deadbeef-dead-beef-dead-beefdeadbeef"

EFI_PROVIDER ?= "rmc-boot"
EFI_PROVIDER_x86-x32 = "grub-efi"

# Add general MACHINEOVERRIDE for meta-intel
MACHINEOVERRIDES =. "intel-x86-common:"

# Definition of (more or less) suitable virtual machines.
require conf/machine/include/qemuboot-intel.inc

# Ensure that the extra tools needed by qemu are built when building images
# and tweak machine definition to make the result more usable under qemu.
require conf/machine/include/qemu-intel.inc
