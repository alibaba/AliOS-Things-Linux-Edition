UMMARY = "Native Linux KVM tool"
DESCRIPTION = "kvmtool is a lightweight tool for hosting KVM guests."

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=fcb02dc552a041dee27e4b85c7396067"

DEPENDS = "dtc libaio zlib"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/will/kvmtool.git \
              file://0001-fix-kvmtool-build-failed.patch \
            "

SRCREV = "9a0069409a54db96e24bd2bb6a39a7ab673fd4be"
PV = "3.18+git${SRCREV}"

S = "${WORKDIR}/git"

python (){
    d.setVar('LDFLAGS', '')

    if d.getVar('TARGET_ARCH') == 'aarch64':
        d.setVar('EXTRA_OEMAKE', 'ARCH=arm64 V=1')
    else:
        d.setVar('EXTRA_OEMAKE', 'ARCH=' + d.getVar('TARGET_ARCH'))
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/lkvm ${D}${bindir}/
}
