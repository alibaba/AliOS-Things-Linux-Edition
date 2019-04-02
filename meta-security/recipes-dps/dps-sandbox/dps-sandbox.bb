SUMMARY = "DPS SANDBOX"
DESCRIPTION = "Protect IoT Devices more security."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PR = "r0"
PV = '1.0.1'

SRC_URI = "file://sandbox-1.0.1.tar"

INSANE_SKIP_${PN} = "already-stripped"

S = "${WORKDIR}"
DPS_PATH = "/system/dps/"

do_compile() {
}

do_install() {
    install -d ${D}${DPS_PATH}
    cp -fr ${S}/sandbox/*   ${D}${DPS_PATH}
}

do_package_qa() {
}

FILES_${PN} = "/system/"