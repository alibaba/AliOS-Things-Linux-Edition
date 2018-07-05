SUMMARY = "TrouSerS - An open-source TCG Software Stack implementation."
DESCRIPTION = "\
Trousers is an open-source TCG Software Stack (TSS), released under the \
Common Public License. Trousers aims to be compliant with the current (1.1b) \
and upcoming (1.2) TSS specifications available from the Trusted Computing \
Group website: http://www.trustedcomputinggroup.org. \
"
HOMEPAGE = "https://sourceforge.net/projects/trousers"
SECTION = "security/tpm"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8031b2ae48ededc9b982c08620573426"

DEPENDS = "openssl10"
PROVIDES = "${PACKAGES}"

PV = "0.3.14+git${SRCPV}"

SRC_URI = "\
    git://git.code.sf.net/p/trousers/trousers \
    file://fix-deadlock-and-potential-hung.patch \
    file://fix-event-log-parsing-problem.patch \
    file://fix-incorrect-report-of-insufficient-buffer.patch \
    file://trousers-conditional-compile-DES-related-code.patch \
    file://Fix-segment-fault-if-client-hostname-cannot-be-retri.patch \
    file://get-user-ps-path-use-POSIX-getpwent-instead-of-getpwe.patch \
    file://trousers.init.sh \
    file://trousers-udev.rules \
    file://tcsd.service \
    file://tcsd.conf \
"
SRCREV = "de57f069ef2297d6a6b3a0353e217a5a2f66e444"

S = "${WORKDIR}/git"

inherit autotools pkgconfig useradd update-rc.d \
    ${@bb.utils.contains('VIRTUAL-RUNTIME_init_manager', 'systemd', 'systemd', '', d)}

EXTRA_OECONF="--with-gui=none"

PACKAGECONFIG ?= "gmp "
PACKAGECONFIG[gmp] = "--with-gmp, --with-gmp=no, gmp"
PACKAGECONFIG[gtk] = "--with-gui=gtk, --with-gui=none, gtk+"

INITSCRIPT_NAME = "trousers"
INITSCRIPT_PARAMS = "start 99 2 3 4 5 . stop 19 0 1 6 ."

USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM_${PN} = "--system tss"
USERADD_PARAM_${PN} = "--system -M -d /var/lib/tpm -s /bin/false -g tss tss"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "tcsd.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install_append() {
    install -d "${D}${sysconfdir}/init.d"
    install -m 0755 "${WORKDIR}/trousers.init.sh" "${D}${sysconfdir}/init.d/trousers"

    install -m 0600 "${WORKDIR}/tcsd.conf" "${D}${sysconfdir}"
    chown tss:tss "${D}${sysconfdir}/tcsd.conf"

    install -d "${D}${sysconfdir}/udev/rules.d"
    install -m 0644 "${WORKDIR}/trousers-udev.rules" \
        "${D}${sysconfdir}/udev/rules.d/45-trousers.rules"

    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
        install -d "${D}${systemd_unitdir}/system"
        install -m 0644 "${WORKDIR}/tcsd.service" "${D}${systemd_unitdir}/system"
        sed -i -e 's#@SBINDIR@#${sbindir}#g' ${D}${systemd_unitdir}/system/tcsd.service
    fi
}

PACKAGES =+ "\
    libtspi \
    libtspi-dbg \
    libtspi-dev \
    libtspi-doc \
    libtspi-staticdev \
"

FILES_libtspi = "\
    ${libdir}/libtspi.so.* \
"
FILES_libtspi-dbg = "\
    ${libdir}/.debug \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/tspi \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/trspi \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/include/*.h \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/include/tss \
"
FILES_libtspi-dev = "\
    ${includedir} \
    ${libdir}/*.so \
"
FILES_libtspi-doc = "\
    ${mandir}/man3 \
"
FILES_libtspi-staticdev = "\
    ${libdir}/*.la \
    ${libdir}/*.a \
"
FILES_${PN}-dbg += "\
    ${sbindir}/.debug \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/tcs \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/tcsd \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/tddl \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/trousers \
    ${prefix}/src/debug/${PN}/${PV}-${PR}/${PN}-${PV}/src/include/trousers \
"
FILES_${PN}-dev += "${libdir}/trousers"
FILES_${PN} += "${systemd_unitdir}/system/tcsd.service"

CONFFILES_${PN} += "${sysconfig}/tcsd.conf"

BBCLASSEXTEND = "native"
