FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += " \
        file://CVE-2017-12883.patch \
        file://CVE-2017-12837.patch \
        file://CVE-2018-6798-1.patch \
        file://CVE-2018-6798-2.patch \
        file://CVE-2018-6797.patch \
        file://CVE-2018-6913.patch \
        file://CVE-2018-12015.patch \
        file://0001-scalar-reverse-extend-stack-if-no-arg.patch \
        "
