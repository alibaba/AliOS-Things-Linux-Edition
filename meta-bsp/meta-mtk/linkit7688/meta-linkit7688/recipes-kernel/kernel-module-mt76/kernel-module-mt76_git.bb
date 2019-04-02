DESCRIPTION = "Openwrt MT76 WLAN driver."
SECTION = "Openwrt WLAN drivers."
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "git://github.com/openwrt/mt76.git;protocol=git;rev=68b0cf17efe32623efd2a46d33b0b551bb78cbbe \
file://Makefile.patch \
"

inherit module

# Modify these as desired
PV = "1.0+git${SRCPV}"
#SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

DEPENDS = "kernel-module-backports"
RDEPENDS_${PN} = "kernel-module-backports"

FILES_${PN}${KERNEL_MODULE_PACKAGE_SUFFIX} += "/lib/firmware/*"

KERNEL_MODULE_AUTOLOAD += "kernel-module-backports kernel-module-mt76 kernel-module-mt7603e kernel-module-mt76x2e"

EXTRA_OEMAKE += "NOSTDINC_FLAGS="-I${S} -I${STAGING_KERNEL_DIR}/usr/include/mac80211-backport/uapi -I${STAGING_KERNEL_DIR}/usr/include/mac80211-backport -I${STAGING_KERNEL_DIR}/usr/include/mac80211/uapi -I${STAGING_KERNEL_DIR}/usr/include/mac80211 -include ${STAGING_KERNEL_DIR}/usr/include/mac80211-backport/backport/autoconf.h -include ${STAGING_KERNEL_DIR}/usr/include/mac80211-backport/backport/backport.h""

do_install_append() {
	install -d ${D}/lib/firmware
	cp \
		${S}/firmware/mt7662_rom_patch.bin \
		${S}/firmware/mt7662.bin \
		${D}/lib/firmware

	cp \
		${S}/firmware/mt7628_e1.bin \
		${S}/firmware/mt7628_e2.bin \
		${S}/firmware/mt7603_e1.bin \
		${S}/firmware/mt7603_e2.bin \
		${D}/lib/firmware

}
