SUMMARY = "Applications and Scripts for libyami."
DESCRIPTION = "Applications and Scripts for libyami."

HOMEPAGE = "https://github.com/01org/libyami-utils"
BUGTRACKER = "https://github.com/01org/libyami-utils/issues/new"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

SRC_URI = "git://github.com/01org/libyami-utils.git \
           file://0001-Fix-build-with-clang.patch \
           "
SRCREV = "b480c0594a7e761a8ccfe6b19b3f9bd0c3d871a1"
S = "${WORKDIR}/git"

DEPENDS = "libva libyami"

EXTRA_OECONF = "--enable-tests-gles --disable-md5"

inherit autotools pkgconfig distro_features_check

REQUIRED_DISTRO_FEATURES = "opengl"

PACKAGECONFIG = "${@bb.utils.filter('DISTRO_FEATURES', 'x11', d)}"

# --enable-x11 needs libva-x11
# gles-tests fail to build without x11: see https://github.com/01org/libyami-utils/issues/91
PACKAGECONFIG[x11] = "--enable-x11 --enable-tests-gles,--disable-x11 --disable-tests-gles, virtual/libx11"

UPSTREAM_CHECK_URI = "http://github.com/01org/libyami-utils/releases"
UPSTREAM_CHECK_REGEX = "(?P<pver>\d+(\.\d+)+)"
