SUMMARY = "DPS SDK"
DESCRIPTION = "Protect IoT Devices more security."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PR = "r0"
PV = '1.0.1'

SRC_URI = "file://dps-1.0.1.tar"
SRC_URI += "file://domain-1.0.1.tar"

# systemd init or sysvinit
inherit ${@ 'systemd' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'update-rc.d' }
inherit bin_package

INSANE_SKIP_${PN} = "already-stripped"

S = "${WORKDIR}"

DPS_PATH = "/system/dps/"
SYSTEM_BIN = "/system/bin/"

LINKER_PATH = "${DPS_PATH}bin/linker"
LINKER64_PATH = "${DPS_PATH}bin/linker64"

LD_SO_PRELOAD = "${D}${sysconfdir}/ld.so.preload"

do_compile() {
}

do_install() {
    #install -f ${LD_SO_PRELOAD}
    install -d ${D}${SYSTEM_BIN}
    install -d ${D}${DPS_PATH}
    install -d ${D}${sysconfdir}

    cp -fr ${S}/dps/*       ${D}${DPS_PATH}
    cp -fr ${S}/domain/*    ${D}${DPS_PATH}
    rm -fr ${D}${DPS_PATH}/startup/

    cd ${D}${SYSTEM_BIN}
    if ${@ 'true' if (d.getVar('TARGET_ARCH') == 'x86_64' or d.getVar('TARGET_ARCH') == 'aarch64') else 'false' }; 
    then
		ln -sf ../dps/bin/linker64  linker64
        echo ${LINKER64_PATH} > ${LD_SO_PRELOAD}
    else
        ln -sf ../dps/bin/linker    linker
        echo ${LINKER_PATH} > ${LD_SO_PRELOAD}
	fi

    if ${@ 'true' if (d.getVar('VIRTUAL-RUNTIME_init_manager') == 'systemd') else 'false' }; then
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${S}/dps/startup/dpsd.service   ${D}${systemd_system_unitdir}
    else
        install -d ${D}/${INIT_D_DIR}
        install -m 0755 ${S}/dps/startup/dpsd.sh        ${D}/${INIT_D_DIR}/dpsd
    fi
}

do_package_qa() {
}

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "dpsd"
INITSCRIPT_PARAMS = "start 99 2 3 4 5 . stop 01 0 1 6 ."

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "dpsd.service"
SYSTEMD_AUTO_ENABLE = "enable"

FILES_${PN} = "${sysconfdir} ${INIT_D_DIR} /system/"