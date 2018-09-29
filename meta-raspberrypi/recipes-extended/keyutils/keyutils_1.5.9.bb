SUMMARY = "Linux Key Management Utilities"
DESCRIPTION = "Keyutils is a set of utilities for managing the key retention \
facility in the kernel, which can be used by filesystems, block devices and \
more to gain and retain the authorization and encryption keys required to \
perform secure operations."
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENCE.GPL;md5=5f6e72824f5da505c1f4a7197f004b45"

PR = "r1"

SRCREV = "9209a0c8fd63afc59f644e078b40cec531409c30"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/dhowells/keyutils.git;protocol=git \
          file://0001-keyutils-Use-libdir-from-the-Yocto-environment.patch \
          file://0001-Include-limits.h-for-UINT_MAX.patch \
          "

S = "${WORKDIR}/git"

inherit autotools-brokensep

INSTALL_FLAGS = "BINDIR=/usr/bin SBINDIR=/usr/sbin DESTDIR=${D} LIB_DIR=${libdir}"

do_install() {
    cd ${S} && oe_runmake ${INSTALL_FLAGS} install
}
