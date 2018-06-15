SUMMARY = "Intel Wireless LinuxCore kernel driver"
DESCRIPTION = "Intel Wireless LinuxCore kernel driver"
SECTION = "kernel"
LICENSE = "GPLv2"

REQUIRED_DISTRO_FEATURES = "wifi"

LIC_FILES_CHKSUM = "file://${S}/COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

inherit module

# For some iwfwifi LinuxCore supported wireless chips, the best/latest
# firmware blobs are found in the iwlwifi's linux-firmware.git fork.
#
# See: https://wireless.wiki.kernel.org/en/users/drivers/iwlwifi/core_release
#
# When updating this recipe, ensure that the proper firmware is included from
# either the linux-firmware or iwlwifi-firmware repos.

PV = "30"
SRCREV = "b31221a99488021300e7f89d2ecf9bdd2bc52dd2"

# Add a patch for Intel's Production Kernel as it's got a backport of HRTimers
PK_PATCH = "${@bb.utils.contains('PREFERRED_PROVIDER_virtual/kernel','linux-intel','file://0001-hrtimer-fix-version-numbers-because-production-kerne.patch','',d)}"

SRC_URI = " \
           git://git.kernel.org/pub/scm/linux/kernel/git/iwlwifi/backport-iwlwifi;branch=release/LinuxCore${PV} \
           file://0001-Makefile.real-skip-host-install-scripts.patch \
           ${PK_PATCH} \
           file://iwlwifi.conf \
          "

S = "${WORKDIR}/git"

EXTRA_OEMAKE = "INSTALL_MOD_PATH=${D} KLIB_BUILD=${KBUILD_OUTPUT}"

do_configure() {
	CC=gcc CFLAGS= LDFLAGS= make defconfig-iwlwifi-public KLIB_BUILD=${KBUILD_OUTPUT}
}

MODULES_INSTALL_TARGET="install"

do_install_append() {
	## install configs and service scripts
	install -d ${D}${sysconfdir}/modprobe.d
	install -m 0644 ${WORKDIR}/iwlwifi.conf ${D}${sysconfdir}/modprobe.d
}

SYSTEMD_AUTO_ENABLE_${PN} = "enable"

RDEPENDS_${PN} = "linux-firmware-iwlwifi"

KERNEL_MODULE_AUTOLOAD_append_core2-32-intel-common = " iwlwifi"
KERNEL_MODULE_AUTOLOAD_append_corei7-64-intel-common = " iwlwifi"

KERNEL_MODULE_PACKAGE_PREFIX = "backport-iwlwifi"
