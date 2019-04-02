DESCRIPTION = "EEMBC CoreMark"
LICENSE = "Apache-2.0"
SRC_URI="git://github.com/eembc/coremark.git"
SRCREV="e05357029445299f94d63d69a281ff0e3583f307"

S="${WORKDIR}/git"
LIC_FILES_CHKSUM = "file://${S}/LICENSE.md;md5=0a18b17ae63deaa8a595035f668aebe1"

def get_subdir(d):
    if '64' in d.getVar('MACHINE'):
        return "linux64"
    else:
        return "linux"
SUBDIR = "${@get_subdir(d)}"

EXTRA_OEMAKE = "'CC=${CC}' 'LD=${LD}' 'PORT_DIR=${SUBDIR}'"
TARGET_CC_ARCH += "${LDFLAGS}"

do_compile () {
    oe_runmake compile
}

do_install () {
    install -d ${D}/opt/coremark/
    install -m 0755 ${S}/coremark.exe ${D}/opt/coremark/
}
FILES_${PN} += "/opt/*"
