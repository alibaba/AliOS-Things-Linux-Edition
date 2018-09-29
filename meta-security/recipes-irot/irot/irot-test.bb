SUMMARY = "irot test"
DESCRIPTION = "irot test, including km test"

PR='r0'
PV='1.1'
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

FILESEXTRAPATHS_prepend := "${THISDIR}:"
SRC_URI = "file://test/irot-test-1.1.tar.gz"

DEPENDS += "irot"

board_arch = "${TARGET_ARCH}"

EXTRA_OEMAKE = "'CC=${CC}' 'board_arch=${board_arch}' 'RANLIB=${RANLIB}' 'AR=${AR}' 'CFLAGS=${CFLAGS} -I${STAGING_DIR_TARGET}/usr/include/tfs -L${STAGING_DIR_TARGET}/usr/lib' \
'LDFLAGS=${LDFLAGS} -L${STAGING_DIR_TARGET}/usr/lib' 'BUILDDIR=${S}'"

do_install() {
    install -m 0777 -d ${D}/var/tfs/test
    install -m 0755 km_test ${D}/var/tfs/test
}

do_package_qa() {
}

FILES_${PN} = "/var/tfs"

