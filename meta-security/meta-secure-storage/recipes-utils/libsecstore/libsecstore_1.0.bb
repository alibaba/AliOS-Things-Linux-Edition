SUMMARY = " Secure store library and manager application"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = " \
           file://libsecstore-1.0 \
           file://secstore-manager.service \
           file://secstore-manager.sh \
          "

# systemd init or sysvinit
inherit ${@ 'systemd' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'update-rc.d' }

DEPENDS = "openssl keyutils ecryptfs-utils libgcrypt"

CFLAGS += "-DSECSTORE_DATA_MAX_LENGTH=65536"

do_configure () {
    :
}

do_compile () {
    oe_runmake
}

do_install () {
    install -d ${D}${bindir}
    install -d ${D}${libdir}
    install -d ${D}${includedir}
    oe_runmake install 'DESTDIR=${D}'
    if ${@ 'true' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'false' }; then
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/secstore-manager.service ${D}${systemd_system_unitdir}
    else
        install -d ${D}/${INIT_D_DIR}
        install -m 0755 -T ${WORKDIR}/secstore-manager.sh ${D}/${INIT_D_DIR}/secstore-manager
    fi

    install -d ${D}/secstore
    install -d ${D}/secstore/shared
    install -d -m 0777 ${D}/secstore/shared/secret
    install -d -m 0777 ${D}/secstore/shared/.secret

    install -d -m 0700 ${D}/secstore/root
    install -d ${D}/secstore/root/secret
    install -d ${D}/secstore/root/.secret
}

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "secstore-manager"
INITSCRIPT_PARAMS = "start 60 S ."

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "secstore-manager.service"
SYSTEMD_AUTO_ENABLE = "enable"

FILES_${PN} += "/secstore/*"
FILES_${PN} += "${libdir}"
