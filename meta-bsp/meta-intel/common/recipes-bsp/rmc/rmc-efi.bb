SUMMARY = "RMC (Runtime Machine Configuration) EFI library"

DESCRIPTION = "The RMC EFI library adds RMC support to existing EFI bootloaders"

LICENSE = "MIT"

LIC_FILES_CHKSUM = "file://COPYING;md5=ade413c694d3aaefc9554b24a8814ee8"

SRC_URI = "git://git.yoctoproject.org/rmc"

SRCREV = "027ac76f642dcab1a9f237a00f03a3a714bd04b9"

S = "${WORKDIR}/git"

COMPATIBLE_HOST = "(x86_64.*|i.86.*)-linux*"

TARGET_CFLAGS +="-Wl,--hash-style=both"

EXTRA_OEMAKE = "RMC_INSTALL_PREFIX=${D}/${prefix} \
                RMC_INSTALL_LIB_PATH=${D}${libdir} \
                RMC_INSTALL_HEADER_PATH=${D}${includedir}/rmc"

SECURITY_CFLAGS_remove_class-target = "-fstack-protector-strong"
SECURITY_CFLAGS_append_class-target = " -fno-stack-protector"

python () {
	ccargs = d.getVar('TUNE_CCARGS').split()
	if '-mx32' in ccargs:
		ccargs.remove('-mx32')
		ccargs.append('-m64')
		d.setVar('TUNE_CCARGS', ' '.join(ccargs))
}

do_compile() {
	oe_runmake -f Makefile.efi
}

do_install() {
	oe_runmake -f Makefile.efi install
}
