DESCRIPTION = "Intel Local Manageability Service allows applications \
to access the Intel Active Management Technology (AMT) firmware via \
the Intel Management Engine Interface (MEI)."
HOMEPAGE = "http://software.intel.com/en-us/articles/download-the-latest-intel-amt-open-source-drivers"

LICENSE = "BSD_LMS"

PR = "r0"
BPN="lms"
SRC_URI = "http://software.intel.com/sites/default/files/${BPN}-${PV}.tar.gz \
           file://readlink-declaration.patch \
           file://0001-Protocol.cpp-Add-whitespace-for-gcc6-compile-error.patch \
           file://0001-Include-sys-select.h-for-fd_set.patch \
           file://0002-Use-proper-netinet-in.h-API.patch \
           file://0003-Fix-device-file-referance-to-dev-mei0-remove-select.patch \
           file://0004-Intel-AMT-ME-real-time-notification-infra.patch \
           "

FILES_${PN} += "${datadir}/xml/AMTAlerts.xml"

COMPATIBLE_HOST = '(i.86|x86_64).*-linux'

LIC_FILES_CHKSUM = "file://COPYING;md5=ec77c894e8a1a89fa07aed2c76680ab8"

SRC_URI[md5sum] = "3cbd027a0e6e9ced8238478b24cde3c6"
SRC_URI[sha256sum] = "7077db6f2f381e67cb37565b20c40ff0c7d3f98f014e65622a4b4b66c2b1d637"

inherit autotools update-rc.d

INITSCRIPT_NAME = "lms8"
INITSCRIPT_PARAMS = "defaults"


do_install_append () {
	mv ${D}/${sbindir}/lms ${D}/${sbindir}/lms8
	install -d ${D}${sysconfdir}/init.d
	# The configure script looks at the host to decide where to put init
	# scripts, so move it at the same time as renaming it.
	if test -f ${D}${sysconfdir}/rc.d/init.d/lms ; then
		mv ${D}${sysconfdir}/rc.d/init.d/lms ${D}${sysconfdir}/init.d/${INITSCRIPT_NAME}
	else
		mv ${D}${sysconfdir}/init.d/lms ${D}${sysconfdir}/init.d/${INITSCRIPT_NAME}
	fi
	sed -i 's/^NAME=lms/NAME=lms8/' ${D}${sysconfdir}/init.d/${INITSCRIPT_NAME}
	rmdir ${D}${datadir} || :
}
