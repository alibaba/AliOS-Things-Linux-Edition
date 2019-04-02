DISTRO = "alios"
DISTRO_NAME = "AliOS Things Linux Edition"
DISTRO_CODENAME = "M3"
DISTRO_VERSION = "1.3.0"
HOSTOS_VERSION = "${DISTRO_VERSION}"

MAINTAINER = "iot <iot@alibaba-inc.com>"

TARGET_VENDOR = "-alios"

SDK_VENDOR = "-aliossdk"
SDK_VERSION := "${@'${DISTRO_VERSION}'.replace('snapshot-${DATE}','snapshot')}"

DISTRO_FEATURES ?= "${DISTRO_FEATURES_DEFAULT} ${DISTRO_FEATURES_LIBC}"

PREFERRED_VERSION_linux-yocto ?= "4.9%"

SDK_NAME = "${DISTRO}-${TCLIBC}-${SDK_ARCH}-${IMAGE_BASENAME}-${TUNE_PKGARCH}"
SDKPATH = "/opt/${DISTRO}/${SDK_VERSION}"

TCLIBCAPPEND = ""

QEMU_TARGETS ?= "arm aarch64 i386 mips mipsel mips64 mips64el nios2 ppc x86_64"

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

# Set AliOS Things splash image
SPLASH = "psplash-alios-things"

# Add welcome message in /etc/issue for /sbin/agetty
# create from http://patorjk.com/software/taag/
add_welcom_message_for_agetty() {
echo "       █████╗ ██╗     ██╗ ██████╗ ███████╗     " >> ${IMAGE_ROOTFS}/etc/issue
echo "      ██╔══██╗██║     ██║██╔═══██╗██╔════╝     " >> ${IMAGE_ROOTFS}/etc/issue
echo "      ███████║██║     ██║██║   ██║███████╗     " >> ${IMAGE_ROOTFS}/etc/issue
echo "      ██╔══██║██║     ██║██║   ██║╚════██║     " >> ${IMAGE_ROOTFS}/etc/issue
echo "      ██║  ██║███████╗██║╚██████╔╝███████║     " >> ${IMAGE_ROOTFS}/etc/issue
echo "      ╚═╝  ╚═╝╚══════╝╚═╝ ╚═════╝ ╚══════╝     " >> ${IMAGE_ROOTFS}/etc/issue
echo "████████╗██╗  ██╗██╗███╗   ██╗ ██████╗ ███████╗" >> ${IMAGE_ROOTFS}/etc/issue
echo "╚══██╔══╝██║  ██║██║████╗  ██║██╔════╝ ██╔════╝" >> ${IMAGE_ROOTFS}/etc/issue
echo "   ██║   ███████║██║██╔██╗ ██║██║  ███╗███████╗" >> ${IMAGE_ROOTFS}/etc/issue
echo "   ██║   ██╔══██║██║██║╚██╗██║██║   ██║╚════██║" >> ${IMAGE_ROOTFS}/etc/issue
echo "   ██║   ██║  ██║██║██║ ╚████║╚██████╔╝███████║" >> ${IMAGE_ROOTFS}/etc/issue
echo "   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝" >> ${IMAGE_ROOTFS}/etc/issue
}
ROOTFS_POSTPROCESS_COMMAND += "add_welcom_message_for_agetty; "


require conf/distro/include/no-static-libs.inc
require conf/distro/include/yocto-uninative.inc
INHERIT += "uninative"

