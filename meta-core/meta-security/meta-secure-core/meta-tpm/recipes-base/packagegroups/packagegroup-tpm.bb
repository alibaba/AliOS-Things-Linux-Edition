DESCRIPTION = "Basic packagegroup for TCG TSS and utilities that use it."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "\
    file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302 \
"

inherit packagegroup

RDEPENDS_${PN} = "\
    trousers \
    tpm-tools \
    tpm-quote-tools \
    openssl-tpm-engine \
    rng-tools \
"

RRECOMMENDS_${PN} = "\
    kernel-module-tpm-rng \
    kernel-module-tpm-tis \
    kernel-module-tpm-atmel \
    kernel-module-tpm-infineon \
"
