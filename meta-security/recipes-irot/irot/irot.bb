SUMMARY = "iot root of trust"
DESCRIPTION = "iot root of trust, including km and irot"

PR='r0'
PV='1.1'
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

FILESEXTRAPATHS_prepend := "${THISDIR}:"
SRC_URI = "file://source/irot-1.1.tar.gz"

DEPENDS += "dbus"

board_arch = "${TARGET_ARCH}"

EXTRA_OEMAKE = "'CC=${CC}' 'board_arch=${board_arch}' 'RANLIB=${RANLIB}' 'AR=${AR}' 'CFLAGS=${CFLAGS} -I${STAGING_DIR_TARGET}/${includedir}/dbus-1.0 \
-I${STAGING_DIR_TARGET}/usr/lib/dbus-1.0/include ' 'BUILDDIR=${S}'"

# systemd init or sysvinit
inherit ${@ 'systemd' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'update-rc.d' }
inherit useradd

USERADD_PACKAGES = "${PN}"
USERADD_PARAM_${PN} = "-r tfs"
USERADD_PARAM_${PN}-dev = "-r tfs"
USERADD_PARAM_${PN}-staticdev = "-r tfs"

do_install() {
    install -d ${D}${bindir}
    install -d ${D}/etc
    install -m 0755 -o tfs -g tfs -d ${D}/usr/.security
    install -m 0755 -o tfs -g tfs -d ${D}/usr/include/tfs
    install -m 0755 -o tfs -g tfs -d ${D}/${libdir}

    #install include file
    install -m 0644 -o tfs -g tfs ${S}/include/km.h ${D}/${includedir}/tfs
    install -m 0644 -o tfs -g tfs ${S}/include/irot.h ${D}/${includedir}/tfs
    install -m 0644 -o tfs -g tfs ${S}/crypto/include/ali_crypto.h ${D}/${includedir}/tfs
    install -m 0644 -o tfs -g tfs ${S}/crypto/include/ali_crypto_types.h ${D}/${includedir}/tfs
    install -m 0644 -o tfs -g tfs ${S}/crypto/include/crypto.h ${D}/${includedir}/tfs

    #install lib file
    install -m 0644 -o tfs -g tfs ${S}/src/libirot.a ${D}${libdir}
    install -m 0644 -o tfs -g tfs ${S}/crypto/lib/${board_arch}/libalicrypto.a ${D}${libdir}
    install -m 0644 -o tfs -g tfs ${S}/crypto/lib/${board_arch}/libmbedcrypto.a ${D}${libdir}

    #include service file
    install -m 0755 -o tfs -g tfs ${S}/src/irot_service ${D}${bindir}
    install -m 0755 -o tfs -g tfs ${S}/tfsbusd/tfsbusd ${D}/usr/bin
    install -m 0644 ${S}/tfsbusd/tfsbusd.conf ${D}/etc

    if ${@ 'true' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'false' }; then
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${S}/irot.service ${D}${systemd_system_unitdir}
        install -m 0644 ${S}/tfsbusd/tfs_dbus.service ${D}${systemd_system_unitdir}
    else
        install -d ${D}/${INIT_D_DIR}
        install -m 0755 -T ${S}/irot.sh ${D}/${INIT_D_DIR}/irot
    fi
}

do_package_qa() {
}

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "irot"
INITSCRIPT_PARAMS = "start 99 2 3 4 5 . stop 01 0 1 6 ."

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "irot.service tfs_dbus.service"
SYSTEMD_AUTO_ENABLE = "enable"

FILES_${PN} = "/usr/.security ${bindir} /etc"
FILES_${PN}-dev = "${includedir}"
FILES_${PN}-staticdev = "${libdir}"

