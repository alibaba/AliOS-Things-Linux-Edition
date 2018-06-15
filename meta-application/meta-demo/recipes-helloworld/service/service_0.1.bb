inherit systemd

SUMMARY = "Install and start a systemd service"
SECTION = "examples"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI  = "file://test_systemd.sh"
SRC_URI += "file://test_systemd.service"

#here we specify the source directory, where we can do all the building and expect sources to be placed
S = "${WORKDIR}"

SYSTEMD_SERVICE_${PN} = "test_systemd.service"


#bitbake task
#created a directory /home/root for target install the script
do_install() {
             install -d ${D}/home/root
             install -m 0755 ${WORKDIR}/test_systemd.sh ${D}/home/root

             install -d ${D}${systemd_system_unitdir}
             install -m 0644 ${WORKDIR}/test_systemd.service ${D}${systemd_system_unitdir}
}

#Pack the path
FILES_${PN} += "/home/root"
FILES_${PN} += "/lib/systemd/system"

REQUIRED_DISTRO_FEATURES= "systemd"
