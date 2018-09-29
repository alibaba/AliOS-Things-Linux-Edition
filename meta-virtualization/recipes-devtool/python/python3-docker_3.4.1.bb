SUMMARY = "A Python library for the Docker Engine API."
HOMEPAGE = "https://github.com/docker/docker-py"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=34f3846f940453127309b920eeb89660"

inherit pypi setuptools3

SRC_URI[md5sum] = "7c3bbe379f7a6a098283ccaa05845117"
SRC_URI[sha256sum] = "ad077b49660b711d20f50f344f70cfae014d635ef094bf21b0d7df5f0aeedf99"

DEPENDS += "${PYTHON_PN}-pip-native"

RDEPENDS_${PN} += " \
	python3-docker-pycreds (>=0.3.0) \
	python3-requests \
	python3-websocket-client \
"
