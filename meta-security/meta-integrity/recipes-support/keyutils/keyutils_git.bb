SUMMARY = "Linux Key Management Utilities"
DESCRIPTION = "Keyutils is a set of utilities for managing the key retention \
facility in the kernel, which can be used by filesystems, block devices and \
more to gain and retain the authorization and encryption keys required to \
perform secure operations."
SECTION = "base"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENCE.GPL;md5=5f6e72824f5da505c1f4a7197f004b45"

PV = "1.5.10+git${SRCPV}"

SRC_URI = "\
    git://git.kernel.org/pub/scm/linux/kernel/git/dhowells/keyutils.git \
    file://keyutils_fix_library_install.patch \
    file://keyutils-fix-the-cflags-for-all-of-targets.patch \
"
SRC_URI_append_arm = " file://keyutils-remove-m32-m64.patch"
SRC_URI_append_aarch64 = " file://keyutils-remove-m32-m64.patch"
SRC_URI_append_mips = " file://keyutils-remove-m32-m64.patch"
SRC_URI_append_mips64 = " file://keyutils-remove-m32-m64.patch"
SRC_URI_append_x86 = " file://keyutils_fix_x86_cflags.patch"
SRC_URI_append_x86-64 = " file://keyutils_fix_x86-64_cflags.patch"
SRC_URI_append_powerpc = "file://keyutils-remove-m32-m64.patch"
SRCREV = "308119c61e94bcc4c710404b9f679e3bb8316713"

S = "${WORKDIR}/git"

inherit autotools-brokensep

EXTRA_OEMAKE = "'CFLAGS=${CFLAGS} -Wall'"

INSTALL_FLAGS = "\
    LIBDIR=${libdir} \
    USRLIBDIR=${libdir} \
    BINDIR=${bindir} \
    SBINDIR=${sbindir} \
    ETCDIR=${sysconfdir} \
    SHAREDIR=${datadir} \
    MANDIR=${mandir} \
    INCLUDEDIR=${includedir} \
    DESTDIR=${D} \
"

do_install() {
    cd ${S} && oe_runmake ${INSTALL_FLAGS} install
}

FILES_${PN} += "${datadir}/request-key-debug.sh"

BBCLASSEXTEND = "native nativesdk"
