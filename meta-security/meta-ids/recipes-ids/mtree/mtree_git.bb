SUMMARY = "BSD directory hierarchy mapping tool"
DESCRIPTION = "mtree compares a file hierarchy against a specification, creates a specification for a file hierarchy, or modifies a specification."

SECTION = "utils"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://COPYING;md5=bb19ea4eac951288efda4010c5c669a8"

PV = "1.0.3+git${SRCPV}"

SRC_URI = "git://github.com/archiecobbs/mtree-port.git \
           file://mtree-getlogin.patch \
           file://configure.ac-automake-error.patch \
           "
SRCREV = "4f3e901aea980fc9a78ac8692fa12a22328b1d4a"

S = "${WORKDIR}/git"

DEPENDS = "openssl"

inherit autotools
