SUMMARY = "A multi-platform support library with a focus on asynchronous I/O"
HOMEPAGE = "https://github.com/libuv/libuv"
BUGTRACKER = "https://github.com/libuv/libuv/issues"

LICENSE = "MIT & BSD-2-Clause & BSD-3-Clause & ISC"
LIC_FILES_CHKSUM = "file://LICENSE;md5=30d33c8196b78acbed849e82bc3b6b18"

SRC_URI = "https://github.com/libuv/${PN}/archive/v${PV}.tar.gz;downloadfilename=${PN}-${PV}.tar.gz"

inherit autotools

do_configure() {
    ${S}/autogen.sh || bbnote "${PN} failed to autogen.sh"
    oe_runconf
}
