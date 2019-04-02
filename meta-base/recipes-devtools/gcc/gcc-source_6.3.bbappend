FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI_append_libc-bionic = " file://0001-Add-supports-for-bionic.patch "
SRC_URI += " \
           file://CVE-2017-11671.patch \
           "
