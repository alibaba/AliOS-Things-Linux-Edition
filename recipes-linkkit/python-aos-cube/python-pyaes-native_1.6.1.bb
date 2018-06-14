SUMMARY = "AliOS Things construction tool"
DESCRIPTION = ""
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=70097d7a871356a2e7f3018a023a09d8"

#https://files.pythonhosted.org/packages/44/66/2c17bae31c906613795711fc78045c285048168919ace2220daa372c7d72/pyaes-1.6.1.tar.gz
SRCNAME = "pyaes"
SRC_URI = "https://files.pythonhosted.org/packages/44/66/2c17bae31c906613795711fc78045c285048168919ace2220daa372c7d72/${SRCNAME}-${PV}.tar.gz"

SRC_URI[md5sum] = "20fd5c6e29dcfdd08098e85a859a54ec"
SRC_URI[sha256sum] = "02c1b1405c38d3c370b085fb952dd8bea3fadcee6411ad99f312cc129c536d8f"

UPSTREAM_CHECK_URI = "https://pypi.python.org/pypi/ecdsa/"

S = "${WORKDIR}/${SRCNAME}-${PV}"

inherit setuptools native pythonnative

DEPENDS = "python-native python-setuptools-native python-requests-native"

#do_install_append() {
#    create_wrapper ${D}${bindir}/aos PYTHONPATH='${STAGING_DIR_HOST}/${PYTHON_SITEPACKAGES_DIR}'
#}