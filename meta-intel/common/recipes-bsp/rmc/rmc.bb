SUMMARY = "RMC (Runtime Machine Configuration)"

DESCRIPTION = "RMC project provides a tool and libraries to identify types \
of hardware boards and access any file-based data specific to the board's \
type at runtime in a centralized way. Software (clients) can have a generic \
logic to query board-specific data from RMC without knowing the type of board. \
This make it possible to have a generic software work running on boards which \
require any quirks or customizations at a board or product level. \
"

LICENSE = "MIT"

LIC_FILES_CHKSUM = "file://COPYING;md5=ade413c694d3aaefc9554b24a8814ee8"

SRC_URI = "git://git.yoctoproject.org/rmc"

SRCREV = "027ac76f642dcab1a9f237a00f03a3a714bd04b9"

S = "${WORKDIR}/git"

COMPATIBLE_HOST = "(x86_64.*|i.86.*)-linux*"

TARGET_CFLAGS +="-Wl,--hash-style=both"

EXTRA_OEMAKE = "RMC_INSTALL_PREFIX=${D}/${prefix} \
                RMC_INSTALL_BIN_PATH=${D}${bindir} \
                RMC_INSTALL_LIB_PATH=${D}${libdir} \
                RMC_INSTALL_HEADER_PATH=${D}${includedir}/rmc"

SECURITY_CFLAGS_remove_class-target = "-fstack-protector-strong"
SECURITY_CFLAGS_append_class-target = " -fno-stack-protector"

do_compile_class-target() {
	oe_runmake
}

do_install() {
	oe_runmake install
}

do_install_class-native() {
	install -d ${D}${STAGING_BINDIR_NATIVE}
	install -m 0755 ${S}/src/rmc ${D}${STAGING_BINDIR_NATIVE}
}

BBCLASSEXTEND = "native"
