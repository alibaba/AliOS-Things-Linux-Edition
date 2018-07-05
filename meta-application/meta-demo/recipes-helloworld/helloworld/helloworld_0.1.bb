SUMMARY = "Simple helloworld application"
DESCRIPTION = "A helloworld application, it printes a helloworld message in \
    endless loop at intervals of 9s"
SECTION = "examples"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "file://helloworld.c \
           file://helloworld.sh \
           file://helloworld.service \
           "
# You do need to set the checksum for no-local source, e.g. downloaded from a git repository.
#SRC_URI[md5sum] = ""
#SRC_URI[sha256sum] = ""

S = "${WORKDIR}"

# systemd init or sysvinit
inherit ${@ 'systemd' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'update-rc.d' }

# Here is a little demonstration application. In order to build your package, you may need to inherit
# scons/cmake/autotools bbclasses which are located in meta-yp/meta/classes according to the build
# system of your package.
do_compile() {
    ${CC} helloworld.c ${LDFLAGS} -o helloworld
}

do_install() {
    install -d ${D}${bindir}

    install -m 0755 helloworld ${D}${bindir}
    if ${@ 'true' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'false' }; then
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${S}/helloworld.service ${D}${systemd_system_unitdir}
    else
        install -d ${D}/${INIT_D_DIR}
        install -m 0755 -T ${WORKDIR}/helloworld.sh ${D}/${INIT_D_DIR}/helloworld
    fi
}

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "helloworld"
INITSCRIPT_PARAMS = "start 99 2 3 4 5 . stop 01 0 1 6 ."

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "helloworld.service"
SYSTEMD_AUTO_ENABLE = "enable"

FILES_${PN} = "${bindir} ${sysconfdir}"
