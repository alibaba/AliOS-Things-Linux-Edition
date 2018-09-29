SUMMARY = "secure storage"
DESCRIPTION = "secure storage, including keychain and sst"

PR='r0'
PV='1.1'
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

FILESEXTRAPATHS_prepend := "${THISDIR}:"
SRC_URI = "file://source/keychain-1.1.tar.gz"

DEPENDS += "dbus irot"
Rdepends += "irot"

board_arch = "${TARGET_ARCH}"

EXTRA_OEMAKE = "'CC=${CC}' 'board_arch=${board_arch}' 'RANLIB=${RANLIB}' 'AR=${AR}' 'CFLAGS=${CFLAGS} -I${STAGING_DIR_TARGET}/usr/include/dbus-1.0 \
-I${STAGING_DIR_TARGET}/usr/lib/dbus-1.0/include -I${STAGING_DIR_TARGET}/usr/include/tfs ' \
'LDFLAGS=${LDFLAGS} -L${STAGING_DIR_TARGET}/usr/lib' 'BUILDDIR=${S}'"

# systemd init or sysvinit
inherit ${@ 'systemd' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'update-rc.d' }
inherit useradd

USERADD_PACKAGES = "${PN}"
USERADD_PARAM_${PN} = "-r tfs"
USERADD_PARAM_${PN}-dev = "-r tfs"
USERADD_PARAM_${PN}-staticdev = "-r tfs"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 -o tfs -g tfs -d ${D}/var/.sst
    install -m 0755 -o tfs -g tfs -d ${D}/etc/.sec
    install -m 0755 -o tfs -g tfs -d ${D}/${includedir}/tfs
    install -m 0755 -o tfs -g tfs -d ${D}/${libdir}

    #install include file
    install -m 0644 -o tfs -g tfs ${S}/include/keychain.h ${D}/${includedir}/tfs
    install -m 0644 -o tfs -g tfs ${S}/sst/include/sst.h ${D}/${includedir}/tfs

    #install lib file
    install -m 0644 -o tfs -g tfs ${S}/src/libkeychain.a ${D}${libdir}
    install -m 0644 -o tfs -g tfs ${S}/sst/src/libsst.a ${D}${libdir}

    #include service file
    install -m 0755 -o tfs -g tfs ${S}/src/keychain_service ${D}${bindir}
    install -m 0755 -o tfs -g tfs ${S}/deploy/deploy_sst ${D}${bindir}

    if ${@ 'true' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'false' }; then
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${S}/keychain.service ${D}${systemd_system_unitdir}
        install -m 0644 ${S}/deploy/deploy.service ${D}${systemd_system_unitdir}
    else
        install -d ${D}/${INIT_D_DIR}
        install -m 0755 -T ${S}/keychain.sh ${D}/${INIT_D_DIR}/keychain
    fi
}

do_package_qa() {
}

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "keychain"
INITSCRIPT_PARAMS = "start 99 2 3 4 5 . stop 01 0 1 6 ."

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "keychain.service deploy.service"
SYSTEMD_AUTO_ENABLE = "enable"

FILES_${PN} = "/var/.sst /etc/.sec ${bindir} /etc"
FILES_${PN}-dev = "${includedir}"
FILES_${PN}-staticdev = "${libdir}"

