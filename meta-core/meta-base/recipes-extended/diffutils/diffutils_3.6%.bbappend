LICENSE = "GPLv3+"
LIC_FILES_CHKSUM = "file://COPYING;md5=d32239bcb673463ab874e80d47fae504"

FILESEXTRAPATHS_prepend := "${THISDIR}/diffutils-3.6:"

SRC_URI_append_libc-glibc += "file://fix_build_error_with_glibc_2.24.patch"
