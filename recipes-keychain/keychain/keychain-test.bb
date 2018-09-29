SUMMARY = "keychain test"
DESCRIPTION = "keychain test, including keychain test"

PR='r0'
PV='1.1'
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

FILESEXTRAPATHS_prepend := "${THISDIR}:"
SRC_URI = "file://test/keychain-test-1.1.tar.gz"

DEPENDS += "dbus keychain irot"

board_arch = "${TARGET_ARCH}"

EXTRA_OEMAKE = "'CC=${CC}' 'board_arch=${board_arch}' 'RANLIB=${RANLIB}' 'AR=${AR}' 'CFLAGS=${CFLAGS} -I${STAGING_DIR_TARGET}/usr/include/dbus-1.0 \
-I${STAGING_DIR_TARGET}/usr/lib/dbus-1.0/include -I${STAGING_DIR_TARGET}/usr/include/tfs' \
'LDFLAGS=${LDFLAGS} -L${STAGING_DIR_TARGET}/usr/lib' 'BUILDDIR=${S}'"

do_install() {
    install -m 0777 -d ${D}/var/tfs/test
    install -m 0755 sec_sst_test_basic ${D}/var/tfs/test
    install -m 0755 sec_sst_test_get ${D}/var/tfs/test
    install -m 0755 sec_sst_test_store ${D}/var/tfs/test
    install -m 0755 sec_sst_test_migration ${D}/var/tfs/test
    install -m 0755 sec_sst_test_performance ${D}/var/tfs/test
    install -m 0755 sec_sst_test_same_uid ${D}/var/tfs/test
    install -m 0755 sec_sst_test_domain ${D}/var/tfs/test
    install -m 0755 sec_sst_test_delete ${D}/var/tfs/test
    install -m 0755 sec_sst_test_func ${D}/var/tfs/test
}

do_package_qa() {
}

FILES_${PN} = "/var/tfs"

