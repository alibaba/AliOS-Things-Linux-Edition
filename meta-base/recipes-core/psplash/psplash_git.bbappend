FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

# NB: this is only for the main logo image; if you add multiple images here,
# poky will build multiple psplash packages with 'outsuffix' in name for
# each of these ...
SPLASH_IMAGES = "file://alios-things.png;outsuffix=default"

# Set psplash-alios-things outsuffix
SPLASH_IMAGES_append = "file://alios-things.png;outsuffix=alios-things"
ALTERNATIVE_PRIORITY_psplash-alios-things[psplash] = "10"
# Set SPLASH in machine or distro config: SPLASH = "psplash-alios-things"

# Systemd Unit: https://www.freedesktop.org/software/systemd/man/systemd.unit.html
# System bootup process: https://www.freedesktop.org/software/systemd/man/bootup.html
# bootup simple as 3 parts:
#  kernel --> systemd init --> local-fs-pre.target
#  --> sysinit.target (start psplash) --> basic.target (update progress) --> multi-user.target (quit psplash)
#  --> default.target

SRC_URI_append = "\
           file://psplash.service \
           file://psplash.path \
           file://psplash-update.service \
           file://psplash-stop.service \
           "

PACKAGE_WRITE_DEPS_append = " ${@bb.utils.contains('DISTRO_FEATURES','systemd','systemd-systemctl-native','',d)}"

do_install_append() {
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/psplash.service ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/psplash.path ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/psplash-update.service ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/psplash-stop.service ${D}${systemd_system_unitdir}
    fi
}

pkg_postinst_${PN} () {
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        if [ -n "$D" ]; then
            OPTS="--root=$D"
        fi
        systemctl $OPTS enable psplash.service
        systemctl $OPTS enable psplash-update.service
        systemctl $OPTS enable psplash-stop.service
    fi
}

# Set in Sysvinit Runlevel 1: https://fedoraproject.org/wiki/SysVinit_to_Systemd_Cheatsheet
# For more information on valid parameters, please see the update-rc.d manual page at
# http://www.tin.org/bin/man.cgi?section=8&topic=update-rc.d.
# need /proc /mnt fs, star 7 S after /etc/rcS.d/S06checkroot.sh
INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "psplash.sh"
INITSCRIPT_PARAMS = "start 7 S . stop 20 0 1 6 ."

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} += "psplash.service"
SYSTEMD_SERVICE_${PN} += "psplash.path"
SYSTEMD_SERVICE_${PN} += "psplash-update.service"
SYSTEMD_SERVICE_${PN} += "psplash-stop.service"
SYSTEMD_AUTO_ENABLE = "enable"

# FILES_${PN} += "${bindir} ${sysconfdir}"
FILES_${PN} += "${systemd_system_unitdir}/psplash.service"
FILES_${PN} += "${systemd_system_unitdir}/psplash.path"
FILES_${PN} += "${systemd_system_unitdir}/psplash-update.service"
FILES_${PN} += "${systemd_system_unitdir}/psplash-stop.service"
