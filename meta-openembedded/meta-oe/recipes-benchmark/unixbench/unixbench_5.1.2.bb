DESCRIPTION = "The BYTE UNIX Benchmarks"
LICENSE = "GPLv2"
RDEPENDS_${PN} = "perl"
SRC_URI="git://github.com/kdlucas/byte-unixbench.git"
SRCREV="5031788bb2c31dc25cf3045a571f2587d223cc59"

S="${WORKDIR}/git/UnixBench"
LIC_FILES_CHKSUM = "file://${S}/../LICENSE.txt;md5=b234ee4d69f5fce4486a80fdaf4a4263"

EXTRA_OEMAKE = "'CC=${CC}' 'OPTON=-O3 -ffast-math'"
TARGET_CC_ARCH += "${LDFLAGS}"

do_install () {
    install -d ${D}/opt/unixbench/
    cp -rf ${S} ${D}/opt/unixbench/
}
FILES_${PN} += "/opt/*"
