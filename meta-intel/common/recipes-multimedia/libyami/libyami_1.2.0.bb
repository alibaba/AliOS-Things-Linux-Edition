SUMMARY = "Yami is media infrastructure base on libva"
DESCRIPTION = "Yet Another Media Infrastructure \
light weight hardware codec library base on VA-API "

HOMEPAGE = "https://github.com/01org/libyami"
BUGTRACKER = "https://github.com/01org/libyami/issues/new"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=3b83ef96387f14655fc854ddc3c6bd57"

SRC_URI = "git://github.com/01org/libyami.git;branch=apache \
           file://0001-bitWriter.cpp-Delete-unused-CACHEBYTES.patch \
           file://0002-typecast-index-from-size_t-to-int.patch \
           file://0003-Add-Wno-invalid-offsetof-to-compiler-commandline.patch \
           file://0004-Typecast-POWER32SUB2-to-uint8_t.patch \
           file://0005-move-c-definitions-out-of-extern-C-block.patch \
           file://0006-Avoid-namespace-conflicts-by-adding-explicit-using-n.patch \
           file://0007-Delete-unused-variables.patch \
           file://0008-NalUnit-is-declared-in-different-namespace.patch \
           file://0009-Fix-clang-warnings.patch \
"
SRCREV = "c884fc3facbf1a0adf2082a1cf95cb146300a5ca"
S = "${WORKDIR}/git"

PACKAGECONFIG ??= "${@bb.utils.contains("DISTRO_FEATURES", "x11", "x11", "", d)}"
PACKAGECONFIG[x11] = "--enable-x11,--disable-x11,virtual/libx11 libxrandr libxrender"

DEPENDS = "libva"
inherit autotools pkgconfig distro_features_check

REQUIRED_DISTRO_FEATURES = "opengl"

UPSTREAM_CHECK_URI = "https://github.com/01org/libyami/releases"
UPSTREAM_CHECK_REGEX = "(?P<pver>\d+(\.\d+)+)"
