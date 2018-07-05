DESCRIPTION = "Screen-scraping library"
HOMEPAGE = "https://pypi.python.org/pypi/beautifulsoup4/"
SECTION = "devel/python"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://COPYING.txt;md5=39dacabe5494f61c8680f6fa7323b596"

SRCNAME = "beautifulsoup4"

SRC_URI = "\
    https://pypi.python.org/packages/source/b/${SRCNAME}/${SRCNAME}-${PV}.tar.gz \
"

SRC_URI[md5sum] = "8fbd9a7cac0704645fa20d1419036815"
SRC_URI[sha256sum] = "87d4013d0625d4789a4f56b8d79a04d5ce6db1152bb65f1d39744f7709a366b4"

S = "${WORKDIR}/${SRCNAME}-${PV}"

inherit setuptools

# avoid "error: option --single-version-externally-managed not recognized"
DISTUTILS_INSTALL_ARGS = "\
    --root=${D} \
    --prefix=${prefix} \
    --install-lib=${PYTHON_SITEPACKAGES_DIR} \
    --install-data=${datadir} \
"

BBCLASSEXTEND = "native"
