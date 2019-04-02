LINUX_VERSION = "4.9.155"

FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}/${BPN}_${LINUX_VERSION}:"

SRC_URI_append = " \
                 ${@bb.utils.contains('LINUX_VERSION', '4.9.49', 'file://0001-tty-fix-data-race-in-tty_ldisc_ref_wait.patch', '', d)} \
                 file://0002-fix-fbtft-driver-bug.patch \
                 ${@bb.utils.contains('LINUX_VERSION', '4.9.155', ' \
                  file://CVE-2017-9986.patch \
                  file://CVE-2018-10323.patch \
                  file://CVE-2018-8043.patch \
                  file://CVE-2018-1000026-1.patch \
                  file://CVE-2018-1000026-2.patch \
                  file://CVE-2017-18232.patch \
                  file://CVE-2018-7273.patch \
                  file://0001-Fix-LTP-test-failed-on-setgid-and-cve-2017-5669.patch \
                  ', '', d)}"

CFLAGS_append_libc-bionic = " -fuse-ld=bfd"
