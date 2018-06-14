SUMMARY = "AliOS Things Linux Edition Accounts"
DESCRIPTION = "This package provides a built-in account named alios, and create \
its home directory. Please change its password if you want to use the sample account"
SECTION = "accounts"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = ""

S = "${WORKDIR}"

inherit useradd

# !!A Big Warning!!
# Please change the below password for your device if you want to use the sample account.
PASSWORD ?= "alios"
USERADD_PACKAGES = "${PN}"
USERADD_PARAM_${PN} = "-u 1200 -m -d /home/alios -s /bin/sh -P ${PASSWORD} -g root -r alios"

do_compile() {
}

do_install() {
    install -d ${D}/home/alios
    chown -R alios ${D}/home/alios
}

do_package_qa() {
}

FILES_${PN} = "/home/alios"
