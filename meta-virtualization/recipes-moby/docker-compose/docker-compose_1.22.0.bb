SUMMARY = "Multi-container orchestration for Docker"
HOMEPAGE = "https://www.docker.com/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=435b266b3899aa8a959f17d41c56def8"

inherit pypi setuptools3

SRC_URI[md5sum] = "2c6030d4f7267a583a16debfd493e4a7"
SRC_URI[sha256sum] = "915cdd0ea7aff349d27a8e0585124ac38695635201770a35612837b25e234677"

RDEPENDS_${PN} = "\
  ${PYTHON_PN}-cached-property (>=1.2.0) \
  ${PYTHON_PN}-certifi (>=2017.4.17) \
  ${PYTHON_PN}-chardet (>=3.0.4) \
  ${PYTHON_PN}-colorama (>=0.3.9) \
  ${PYTHON_PN}-docker (>=3.4.1)\
  ${PYTHON_PN}-docker-pycreds \
  ${PYTHON_PN}-dockerpty (>=0.4.1) \
  ${PYTHON_PN}-docopt (>=0.6.1) \
  ${PYTHON_PN}-enum (>=1.0.4) \
  ${PYTHON_PN}-idna (>=2.5) \
  ${PYTHON_PN}-jsonschema (>=2.5.1) \
  ${PYTHON_PN}-pyyaml (>=3.10) \
  ${PYTHON_PN}-requests (>=2.6.1) \
  ${PYTHON_PN}-six (>=1.3.0) \
  ${PYTHON_PN}-texttable (>=0.9.0) \
  ${PYTHON_PN}-urllib3 (>=1.12.1) \
  ${PYTHON_PN}-websocket-client (>=0.32.0) \
  ${PYTHON_PN}-pysocks (>=1.5.8) \
  ${PYTHON_PN}-ipaddress (>=1.0.18) \
  ${PYTHON_PN}-terminal \
  libgcc \
  "
