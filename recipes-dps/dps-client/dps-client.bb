SUMMARY = "DPS CLIENT"
DESCRIPTION = "Dps client for IoT devices to connect Aliyun platform."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PR = "r0"
PV = '1.0.1'

SRC_URI = "file://client-1.0.1.tar"

INSANE_SKIP_${PN} = "already-stripped"

S = "${WORKDIR}"
DPS_PATH = "/system/dps/"

do_compile() {
}

do_install() {
    install -d ${D}${DPS_PATH}
    cp -fr ${S}/client/*    ${D}${DPS_PATH}
}

do_package_qa() {
}

FILES_${PN} = "/system/"