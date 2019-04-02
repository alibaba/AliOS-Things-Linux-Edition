FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += " \
        file://CVE-2017-15906.patch;striplevel=3 \
        "
