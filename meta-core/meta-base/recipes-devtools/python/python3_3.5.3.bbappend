FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += " \
        file://CVE-2017-1000158.patch \
        file://CVE-2018-1060_CVE-2018-1061.patch \
        file://CVE-2017-18207.patch \
        "
