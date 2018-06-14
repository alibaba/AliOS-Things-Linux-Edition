SUMMARY = "AliOS Things construction tool"
DESCRIPTION = ""
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=3a885dff6d14e4cd876d9008a09a42de"

#https://files.pythonhosted.org/packages/source/a/esptool/esptool-2.3.1.tar.gz
SRCNAME = "esptool"
SRC_URI = "https://files.pythonhosted.org/packages/cd/68/c28961d88cf50ca6d5de5e4b354dc47f77b9e74d4cd4d5bee4feaa7963b3/${SRCNAME}-${PV}.tar.gz"

SRC_URI[md5sum] = "bdf69c620724ce5a35180cea4ff2f783"
SRC_URI[sha256sum] = "4578cc180b3d9ec27bdc254149a18ec70d4cbb6987700e21d1d9e38fde637131"

UPSTREAM_CHECK_URI = "https://pypi.python.org/pypi/esptool/"

S = "${WORKDIR}/${SRCNAME}-${PV}"

inherit setuptools native pythonnative

DEPENDS = "python-native python-setuptools-native python-requests-native"

#do_install_append() {
#    create_wrapper ${D}${bindir}/aos PYTHONPATH='${STAGING_DIR_HOST}/${PYTHON_SITEPACKAGES_DIR}'
#}