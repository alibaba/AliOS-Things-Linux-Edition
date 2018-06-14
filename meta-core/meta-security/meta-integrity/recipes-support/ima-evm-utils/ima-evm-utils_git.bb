LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

DEPENDS += "openssl attr keyutils"

PV = "1.0+git${SRCPV}"

SRC_URI = "\
    git://git.code.sf.net/p/linux-ima/ima-evm-utils \
    file://0001-Don-t-build-man-pages.patch \
    file://0001-Install-evmctl-to-sbindir-rather-than-bindir.patch \
    file://Fix-the-build-failure-with-openssl-1.1.x.patch \
"
SRCREV = "3e2a67bdb0673581a97506262e62db098efef6d7"

S = "${WORKDIR}/git"

inherit pkgconfig autotools

# Specify any options you want to pass to the configure script using EXTRA_OECONF:
EXTRA_OECONF = ""

FILES_${PN}-dev += "${includedir}"

RDEPENDS_${PN}_class-target += "libcrypto libattr keyutils"

BBCLASSEXTEND = "native nativesdk"
