SUMMARY = "AliOS Things construction tool"
DESCRIPTION = ""
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRCNAME = "aos-cube"
SRC_URI = "https://files.pythonhosted.org/packages/source/a/${SRCNAME}/${SRCNAME}-${PV}.tar.gz"
SRC_URI[md5sum] = "ece8f3c6172ae3f839510d2d1eff4740"
SRC_URI[sha256sum] = "162c320edf80d3930e41e591571a6ebdaa5c92d5946b0292a606cccb6bd9555e"

#SRC_URI[md5sum] = "9f189a8bbdb113cb965ab3d4be43939c"
#SRC_URI[sha256sum] = "cbe8d2c1b2f5fc78d6327f171d549e83d3af69c6000067bd75efc45ea882be52"

UPSTREAM_CHECK_URI = "https://pypi.python.org/pypi/aos-cube/"

S = "${WORKDIR}/${SRCNAME}-${PV}"

inherit setuptools native pythonnative

DEPENDS = "python-native \
            python-setuptools-native \
            python-requests-native \
            python-esptool-native \
            python-pyserial-native \
            python-certifi-native \
            python-urllib3-native\
            python-idna-native \
            python-chardet-native \
            python-ecdsa-native \
            python-pyaes-native "

#do_install_append() {
#    create_wrapper ${D}${bindir}/aos PYTHONPATH='${STAGING_DIR_HOST}/${PYTHON_SITEPACKAGES_DIR}'
#}