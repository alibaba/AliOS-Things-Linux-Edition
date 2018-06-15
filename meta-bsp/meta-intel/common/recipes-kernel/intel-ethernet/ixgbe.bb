SUMMARY="ixgbe kernel driver for Intel Magnolia Park 10GbE"
DESCRIPTION="Intel 10-Gbps Ethernet driver for Magnolia Park"
AUTHOR = "Ong Boon Leong"
HOMEPAGE = "http://www.intel.com/network/connectivity/products/server_adapters.htm"
SECTION = "kernel/network"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/${PN}-${PV}/COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

PV = "5.1.3"
PR = "r0"

SRC_URI = "https://sourceforge.net/projects/e1000/files/ixgbe%20stable/${PV}/ixgbe-${PV}.tar.gz \
           file://0001-ixgbe-src-Makefile-change-make-install-to-make.patch \
           file://0001-ixgbe-skip-host-depmod.patch \
           "

SRC_URI[md5sum] = "ae35ed547aa6a5087672c3f70ce1e676"
SRC_URI[sha256sum] = "9f537d79bddf0a087a17af632d57812d26d26bcfebbd4bdcf10df656ff055bb4"

S = "${WORKDIR}/${PN}-${PV}/src"
SCRIPT_DIR = "${WORKDIR}/${PN}-${PV}/scripts"

EXTRA_OEMAKE='KSRC="${STAGING_KERNEL_BUILDDIR}" KVER="${KERNEL_VERSION}" \
              BUILD_ARCH="${TARGET_ARCH}" PREFIX="${D}" \
              SYSTEM_MAP_FILE="${STAGING_KERNEL_BUILDDIR}/System.map-${KERNEL_VERSION}" INSTALL_MOD_PATH="${D}"'

KERNEL_MODULE_AUTOLOAD_append_intel-core2-32 = " ixgbe"
KERNEL_MODULE_AUTOLOAD_append_intel-corei7-64 = " ixgbe"

inherit module

do_install_append () {
        # Install scripts/set_irq_affinity
        install -d      ${D}/etc/network
        install -m 0755 ${SCRIPT_DIR}/set_irq_affinity  ${D}/etc/network
}

#SSTATE_DUPWHITELIST += "${STAGING_DIR_HOST}/lib/modules/${KERNEL_VERSION}/"

PACKAGES += "${PN}-script"

FILES_${PN}-script += "/etc/network/set_irq_affinity"

#Ignore "ERROR: QA Issue: ixgbe: Files/directories were installed but not shipped"
INSANE_SKIP_${PN} = "installed-vs-shipped"
