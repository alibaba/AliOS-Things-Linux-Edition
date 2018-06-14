SUMMARY = "AliOS Things construction tool"
DESCRIPTION = ""
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=66ffc5e30f76cbb5358fe54b645e5a1d"

#https://files.pythonhosted.org/packages/f9/e5/99ebb176e47f150ac115ffeda5fedb6a3dbb3c00c74a59fd84ddf12f5857/ecdsa-0.13.tar.gz
SRCNAME = "ecdsa"
SRC_URI = "https://files.pythonhosted.org/packages/f9/e5/99ebb176e47f150ac115ffeda5fedb6a3dbb3c00c74a59fd84ddf12f5857/${SRCNAME}-${PV}.tar.gz"

SRC_URI[md5sum] = "1f60eda9cb5c46722856db41a3ae6670"
SRC_URI[sha256sum] = "64cf1ee26d1cde3c73c6d7d107f835fed7c6a2904aef9eac223d57ad800c43fa"

UPSTREAM_CHECK_URI = "https://pypi.python.org/pypi/ecdsa/"

S = "${WORKDIR}/${SRCNAME}-${PV}"

inherit setuptools native pythonnative

DEPENDS = "python-native python-setuptools-native python-requests-native"

#do_install_append() {
#    create_wrapper ${D}${bindir}/aos PYTHONPATH='${STAGING_DIR_HOST}/${PYTHON_SITEPACKAGES_DIR}'
#}