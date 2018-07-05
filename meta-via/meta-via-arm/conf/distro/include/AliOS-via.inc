# Poky based distro file

# This function changes the default tune for machines which are based on armv7a
# to use common tune value
def arm_tune_handler(d):
    features = d.getVar('TUNE_FEATURES', True).split()
    if 'armv7a' in features or 'armv7ve' in features:
        tune = 'armv7athf'
        if 'bigendian' in features:
            tune += 'b'
        if 'vfpv3' in features:
            tune += '-vfpv3'
        if 'vfpv3d16' in features:
            tune += '-vfpv3d16'
        if 'neon' in features:
            tune += '-neon'
        if 'vfpv4' in features:
            tune += '-vfpv4'
    else:
        tune = d.getVar('DEFAULTTUNE', True)
    return tune

DEFAULTTUNE_fslc := "${@arm_tune_handler(d)}"

DISTRO_ARM_INSTRUCTION ?= "thumb"
DISTRO_ARM_INSTRUCTION_armv5 ?= "arm"
ARM_INSTRUCTION_SET_fslc ??= "${DISTRO_ARM_INSTRUCTION}"

# Log information on images and packages
INHERIT += "buildhistory"
BUILDHISTORY_COMMIT = "1"

# Use bluez5 as default.
DISTRO_FEATURES_append = " bluez5"

# The following set the providers to components supported by NXP
# Use i.MX Kernel, U-Boot and Gstreamer 1.0 providers
PREFERRED_PROVIDER_u-boot_mx6dl = "u-boot-imx"
PREFERRED_PROVIDER_u-boot_mx6q = "u-boot-imx"
PREFERRED_PROVIDER_u-boot_mx6sl = "u-boot-imx"
PREFERRED_PROVIDER_u-boot_mx6sx = "u-boot-imx"
PREFERRED_PROVIDER_u-boot_mx6ul = "u-boot-imx"
PREFERRED_PROVIDER_u-boot_mx6ull = "u-boot-imx"
PREFERRED_PROVIDER_u-boot_mx7d = "u-boot-imx"

PREFERRED_PROVIDER_virtual/bootloader_mx6dl = "u-boot-imx"
PREFERRED_PROVIDER_virtual/bootloader_mx6q = "u-boot-imx"
PREFERRED_PROVIDER_virtual/bootloader_mx6sl = "u-boot-imx"
PREFERRED_PROVIDER_virtual/bootloader_mx6sx = "u-boot-imx"
PREFERRED_PROVIDER_virtual/bootloader_mx6ul = "u-boot-imx"
PREFERRED_PROVIDER_virtual/bootloader_mx6ull = "u-boot-imx"
PREFERRED_PROVIDER_virtual/bootloader_mx7d = "u-boot-imx"

PREFERRED_PROVIDER_virtual/kernel_mx6dl = "linux-imx"
PREFERRED_PROVIDER_virtual/kernel_mx6q = "linux-imx"
PREFERRED_PROVIDER_virtual/kernel_mx6sl = "linux-imx"
PREFERRED_PROVIDER_virtual/kernel_mx6sx = "linux-imx"
PREFERRED_PROVIDER_virtual/kernel_mx6ul = "linux-imx"
PREFERRED_PROVIDER_virtual/kernel_mx6ull = "linux-imx"
PREFERRED_PROVIDER_virtual/kernel_mx7d = "linux-imx"

MACHINE_GSTREAMER_1_0_PLUGIN_mx6dl = "imx-gst1.0-plugin"
MACHINE_GSTREAMER_1_0_PLUGIN_mx6q = "imx-gst1.0-plugin"
MACHINE_GSTREAMER_1_0_PLUGIN_mx6sl = "imx-gst1.0-plugin"
MACHINE_GSTREAMER_1_0_PLUGIN_mx6sx = "imx-gst1.0-plugin"
MACHINE_GSTREAMER_1_0_PLUGIN_mx6ul = "imx-gst1.0-plugin"
MACHINE_GSTREAMER_1_0_PLUGIN_mx6ull = "imx-gst1.0-plugin"
MACHINE_GSTREAMER_1_0_PLUGIN_mx7d = "imx-gst1.0-plugin"

DISTRO_FEATURES_remove = "x11 wayland directfb "

DISTRO = "AliOS-via"
DISTRO_NAME = "AliOS Things"
DISTRO_CODENAME = "M1"
DISTRO_VERSION = "1.0.0"
HOSTOS_VERSION = "${DISTRO_VERSION}"

MAINTAINER = "iot <iot@alibaba-inc.com>"

TARGET_VENDOR = "-AliOS"

SDK_VENDOR = "-aliossdk"
SDK_VERSION := "${@'${DISTRO_VERSION}'.replace('snapshot-${DATE}','snapshot')}"

# Override these in poky based distros
POKY_DEFAULT_DISTRO_FEATURES = "largefile opengl ptest multiarch wayland vulkan"
POKY_DEFAULT_EXTRA_RDEPENDS = "packagegroup-core-boot"
POKY_DEFAULT_EXTRA_RRECOMMENDS = "kernel-module-af-packet"

DISTRO_FEATURES ?= "${DISTRO_FEATURES_DEFAULT} ${DISTRO_FEATURES_LIBC} ${POKY_DEFAULT_DISTRO_FEATURES}"

DISTRO_EXTRA_RDEPENDS += " ${POKY_DEFAULT_EXTRA_RDEPENDS}"
DISTRO_EXTRA_RRECOMMENDS += " ${POKY_DEFAULT_EXTRA_RRECOMMENDS}"

SDK_NAME = "${DISTRO}-${TCLIBC}-${SDK_ARCH}-${IMAGE_BASENAME}-${TUNE_PKGARCH}"
SDKPATH = "/opt/${DISTRO}/${SDK_VERSION}"

DISTRO_EXTRA_RDEPENDS += " ${POKY_DEFAULT_EXTRA_RDEPENDS}"
DISTRO_EXTRA_RRECOMMENDS += " ${POKY_DEFAULT_EXTRA_RRECOMMENDS}"

# This hooks into packagegroup-base, so it won't do anything if your images doesn't include packagegroup-base.
DISTRO_EXTRA_RDEPENDS += ""

# This also hooks into packagegroup-base, but isn't mandatory.
DISTRO_EXTRA_RRECOMMENDS += ""

POKYQEMUDEPS = "${@bb.utils.contains("INCOMPATIBLE_LICENSE", "GPL-3.0", "", "packagegroup-core-device-devel",d)}"
DISTRO_EXTRA_RDEPENDS_append_qemuarm = " ${POKYQEMUDEPS}"
DISTRO_EXTRA_RDEPENDS_append_qemuarm64 = " ${POKYQEMUDEPS}"
DISTRO_EXTRA_RDEPENDS_append_qemumips = " ${POKYQEMUDEPS}"
DISTRO_EXTRA_RDEPENDS_append_qemuppc = " ${POKYQEMUDEPS}"
DISTRO_EXTRA_RDEPENDS_append_qemux86 = " ${POKYQEMUDEPS}"
DISTRO_EXTRA_RDEPENDS_append_qemux86-64 = " ${POKYQEMUDEPS}"

TCLIBCAPPEND = ""

QEMU_TARGETS ?= "arm aarch64 i386 mips mipsel mips64 mips64el nios2 ppc x86_64"
# Other QEMU_TARGETS "sh4"

PREMIRRORS ??= "\
bzr://.*/.*   http://downloads.yoctoproject.org/mirror/sources/ \n \
cvs://.*/.*   http://downloads.yoctoproject.org/mirror/sources/ \n \
git://.*/.*   http://downloads.yoctoproject.org/mirror/sources/ \n \
gitsm://.*/.* http://downloads.yoctoproject.org/mirror/sources/ \n \
hg://.*/.*    http://downloads.yoctoproject.org/mirror/sources/ \n \
osc://.*/.*   http://downloads.yoctoproject.org/mirror/sources/ \n \
p4://.*/.*    http://downloads.yoctoproject.org/mirror/sources/ \n \
svn://.*/.*   http://downloads.yoctoproject.org/mirror/sources/ \n"

MIRRORS =+ "\
ftp://.*/.*      http://downloads.yoctoproject.org/mirror/sources/ \n \
http://.*/.*     http://downloads.yoctoproject.org/mirror/sources/ \n \
https://.*/.*    http://downloads.yoctoproject.org/mirror/sources/ \n"

# The CONNECTIVITY_CHECK_URI's are used to test whether we can succesfully
# fetch from the network (and warn you if not). To disable the test set
# the variable to be empty.
# Git example url: git://git.yoctoproject.org/yocto-firewall-test;protocol=git;rev=master
CONNECTIVITY_CHECK_URIS ?= "https://www.aliyun.com/"

SANITY_TESTED_DISTROS ?= " \
            poky-2.4 \n \
            ubuntu-16.04 \n \
            ubuntu-16.10 \n \
            "
#
# OELAYOUT_ABI allows us to notify users when the format of TMPDIR changes in
# an incompatible way. Such changes should usually be detailed in the commit
# that breaks the format and have been previously discussed on the mailing list
# with general agreement from the core team.
#
OELAYOUT_ABI = "12"

# QA check settings - a little stricter than the OE-Core defaults
WARN_TO_ERROR_QA = "already-stripped compile-host-path install-host-path \
                    installed-vs-shipped ldflags pn-overrides rpaths staticdev \
                    useless-rpaths"
WARN_QA_remove = "${WARN_TO_ERROR_QA}"
ERROR_QA_append = " ${WARN_TO_ERROR_QA}"

require conf/distro/include/no-static-libs.inc
require conf/distro/include/yocto-uninative.inc
INHERIT += "uninative"

