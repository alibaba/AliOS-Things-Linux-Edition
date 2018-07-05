require packagegroup-tpm2.inc

RDEPENDS_${PN} += "\
    tpm2-abrmd \
    tpm2.0-tools \
    rng-tools \
"

RRECOMMENDS_${PN} += "\
    kernel-module-tpm-rng \
"
