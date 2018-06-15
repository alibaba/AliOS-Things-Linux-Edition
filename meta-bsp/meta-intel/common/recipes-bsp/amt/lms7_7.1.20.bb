DESCRIPTION = "Intel Local Manageability Service allows applications \
to access the Intel Active Management Technology (AMT) firmware via \
the Intel Management Engine Interface (MEI)."
HOMEPAGE = "http://software.intel.com/en-us/articles/download-the-latest-intel-amt-open-source-drivers"

LICENSE = "BSD_LMS"

PR = "r0"
BPN="lms"
PV_SUB = "25"
SRC_URI = "http://software.intel.com/sites/default/files/m/4/e/a/9/b/37962-${BPN}_${PV}.${PV_SUB}.zip \
           file://atnetworktool-printf-fix.patch \
           file://readlink-declaration.patch \
           file://0001-Protocol.cpp-Add-whitespace-for-gcc6-compile-error.patch \
           file://0001-Include-sys-select.h-for-fd_set.patch \
           file://0002-Use-proper-netinet-in.h-API.patch \
           "

LOCALSRC = "file://${WORKDIR}/outputdir/${BPN}-${PV}-${PV_SUB}.tar.gz"

COMPATIBLE_HOST = '(i.86|x86_64).*-linux'

LIC_FILES_CHKSUM = "file://COPYING;md5=7264184cf88d9f27b719a9656255b47b"

SRC_URI[md5sum] = "687b76e78bfdbcf567c0e842c1fe240a"
SRC_URI[sha256sum] = "cc0457f0044e924794bb1aeae9a72c28666a525cd8a963d0d92970222946e75b"

inherit autotools update-rc.d

INITSCRIPT_NAME = "lms7"
INITSCRIPT_PARAMS = "defaults"

python do_unpack() {
    s = d.getVar('S', True)
    d.setVar('S', '${WORKDIR}/outputdir')
    bb.build.exec_func('base_do_unpack', d)
    # temorarily change SRC_URI for unpack
    src_uri = d.getVar('SRC_URI', True)
    d.setVar('SRC_URI', '${LOCALSRC}')
    d.setVar('S', s)
    bb.build.exec_func('base_do_unpack', d)
    d.setVar('SRC_URI', src_uri)
}


do_install_append () {
	mv ${D}/${sbindir}/lms ${D}/${sbindir}/lms7
	install -d ${D}${sysconfdir}/init.d
	# The configure script looks at the host to decide where to put init
	# scripts, so move it at the same time as renaming it.
	if test -f ${D}${sysconfdir}/rc.d/init.d/lms ; then
		mv ${D}${sysconfdir}/rc.d/init.d/lms ${D}${sysconfdir}/init.d/${INITSCRIPT_NAME}
	else
		mv ${D}${sysconfdir}/init.d/lms ${D}${sysconfdir}/init.d/${INITSCRIPT_NAME}
	fi
	sed -i 's/^NAME=lms/NAME=lms7/' ${D}${sysconfdir}/init.d/${INITSCRIPT_NAME}
	rmdir ${D}${datadir} || :
}
