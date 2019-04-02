SUMMARY = "DPS CLIENT LIBS"
DESCRIPTION = "Dps client libs for IoT devices to connect Aliyun platform."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PR = 'r0'
PV = '1.0'
FILESEXTRAPATHS_prepend := "${THISDIR}:"
SRC_URI = "file://source/dps-client-src-1.0.tar"

S = "${WORKDIR}/dps-client-lib"

INSANE_SKIP_${PN} = "already-stripped"

do_compile() {
    make all
}

do_install() {
    install -d ${D}/system/bin/
    install -m 0755 ${S}/output/client/dps_client ${D}/system/bin/dps_client
}

do_package_qa() {
}

FILES_${PN} = "/system/bin/"