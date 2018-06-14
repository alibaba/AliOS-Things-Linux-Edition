SUMMARY = "Command line utility to extend hash of arbitrary data into a TPMs PCR."
SECTION = "security/tpm"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

DEPENDS = "libtspi"

PV = "0.1+git${SRCPV}"

SRC_URI = "\
    git://github.com/flihp/pcr-extend.git \
"
SRCREV = "c02ad8f628b3d99f6d4c087b402fe31a40ee6316"

S = "${WORKDIR}/git"

inherit autotools

do_compile() {
    oe_runmake -C "${S}"
}

do_install() {
    install -d "${D}${bindir}"
    DESTDIR="${D}" oe_runmake install -C "${S}"
}
