LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://LICENSE;md5=a23a74b3f4caf9616230789d94217acb"

DEPENDS += "attr ima-evm-utils tclap"

SRC_URI = "git://github.com/mgerstner/ima-inspect.git"
SRCREV = "e912be2d2a9fdf30a9693a7fc5d6b2473990a71c"

S = "${WORKDIR}/git"

inherit autotools pkgconfig
