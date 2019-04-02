FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += " \
           file://CVE-2017-14245-14246.patch \
           file://CVE-2017-14634.patch \
           file://CVE-2018-13139.patch \
           file://0001-a-ulaw-fix-multiple-buffer-overflows-432.patch \
           file://CVE-2017-12562.patch \
           "
