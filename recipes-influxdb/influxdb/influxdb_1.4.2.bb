SUMMARY = "Package for AliOS Edge"
DESCRIPTION = "Package for AliOS Edge"
SECTION = "AliOS Edge"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://influxdb-1.4.2.tar.gz"


S = "${WORKDIR}/"

inherit bin_package

INSANE_SKIP_${PN} = "already-stripped"
PACKAGES = "${PN}"

do_install() {
  cp -fr ${S}/influxdb-1.4.2-1/* ${D}
}

do_package_qa() {
}
