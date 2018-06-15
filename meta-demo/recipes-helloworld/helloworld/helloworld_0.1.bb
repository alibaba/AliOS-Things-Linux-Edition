#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Simple helloworld application"
SECTION = "examples"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "file://helloworld.c"

S = "${WORKDIR}"

inherit useradd

PASSWORD ?= "alios"
USERADD_PACKAGES = "${PN}"
USERADD_PARAM_${PN} = "-u 1200 -m -d /home/alios -s /bin/bash -P ${PASSWORD} -r alios"

do_compile() {
    ${CC} helloworld.c -o helloworld
}

do_install() {
    install -d ${D}/home/alios

    install -m 0755 helloworld ${D}/home/alios

    chown -R alios ${D}/home/alios
}

do_package_qa() {
}

FILES_${PN} = "/home/alios"
