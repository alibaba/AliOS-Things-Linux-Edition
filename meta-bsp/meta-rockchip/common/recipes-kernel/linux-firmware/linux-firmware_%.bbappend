FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

# Install addition firmwares
do_install_append() {
	cp -r ${WORKDIR}/firmware ${D}${nonarch_base_libdir}/
}

# For broadcom
SRC_URI += " \
	file://firmware/brcm/brcmfmac43455-sdio.txt \
	file://firmware/brcm/brcmfmac43455-sdio.clm_blob \
"

PACKAGES_prepend += " \
	${PN}-bcm43455 \
"

LICENSE_${PN}-bcm43455 = "Firmware-broadcom_bcm43xx"
LICENSE_${PN}-broadcom-license = "Firmware-broadcom_bcm43xx"

FILES_${PN}-broadcom-license = " \
  ${nonarch_base_libdir}/firmware/LICENCE.broadcom_bcm43xx \
"
FILES_${PN}-bcm43455 = " \
  ${nonarch_base_libdir}/firmware/brcm/brcmfmac43455-sdio.* \
"

RDEPENDS_${PN}-bcm43455 += "${PN}-broadcom-license"
