DESCRIPTION = "ARM Trusted Firmware"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://license.rst;md5=e927e02bca647e14efd87e9e914b2443"

PROVIDES = "virtual/arm-trusted-firmware"

inherit deploy

DEPENDS += "u-boot-mkimage-native u-boot openssl openssl-native optee-os-qoriq"

S = "${WORKDIR}/git"
B = "${WORKDIR}/build"
B2 = "${WORKDIR}/build2"


BRANCH ?= ""
REPO ?= "git://github.com/ARM-software/arm-trusted-firmware.git;protocol=https"
BRANCHARG = "${@['nobranch=1', 'branch=${BRANCH}'][d.getVar('BRANCH', True) != '']}"
SRC_URI = "${REPO};${BRANCHARG} \
    file://0001-layerscape-remove-bl1-and-move-bl2-to-el3.patch \
    file://0002-layerscape-add-sd-boot-support-for-ls1043ardb-board.patch \
    file://0003-layerscape-adjustment-and-optimized-code-on-LS1043A.patch \
    file://0004-layerscape-Initial-TF-A-suppoprt-for-LS1046ardb.patch \
    file://0005-layerscape-Fix-compiling-errors-when-enabling-TSP-as.patch \
    file://0006-layerscape-fix-arguments-passing-from-bl2-to-bl31.patch \
    file://0007-layerscape-change-bl2-and-bl31-ocram-size.patch \
    file://0008-layerscape-complete-plat_crash_print_regs-function.patch \
    file://0001-fix-fiptool-build-error.patch \
"

ATF_BASE_NAME ?= "${PN}-${PKGE}-${PKGV}-${PKGR}-${DATETIME}"
ATF_BASE_NAME[vardepsexclude] = "DATETIME"

COMPATIBLE_MACHINE = "ls1043ardb"
PLATFORM_ls1043ardb = "ls1043"

# requires CROSS_COMPILE set by hand as there is no configure script
export CROSS_COMPILE="${TARGET_PREFIX}"
export ARCH="arm64"
# Let the Makefile handle setting up the CFLAGS and LDFLAGS as it is a standalone application
CFLAGS[unexport] = "1"
LDFLAGS[unexport] = "1"
AS[unexport] = "1"
LD[unexport] = "1"

do_configure() {
	:
}

do_compile() {
       export  LIBPATH="${RECIPE_SYSROOT_NATIVE}"
       cp ${DEPLOY_DIR_IMAGE}/u-boot.bin-atf ${S}
       cp ${DEPLOY_DIR_IMAGE}/u-boot.bin-atf-sd ${S}
       cp ${DEPLOY_DIR_IMAGE}/optee/tee_ls1043ardb.bin ${S}
       install -d ${S}/include/tools_share/openssl
       cp -r ${RECIPE_SYSROOT}/usr/include/openssl/*   ${S}/include/tools_share/openssl
       oe_runmake -C ${S} BUILD_BASE=${B} PLAT=${PLATFORM} bl2
       oe_runmake V=1 -C ${S} BUILD_BASE=${B} PLAT=${PLATFORM} fip BL33=u-boot.bin-atf NEED_BL32=yes BL32=tee_ls1043ardb.bin SPD=opteed
       oe_runmake -C ${S} BUILD_BASE=${B2} PLAT=${PLATFORM}  bl2 BOOT_MODE=sd 
       oe_runmake -C ${S} BUILD_BASE=${B2} PLAT=${PLATFORM} fip BL33=u-boot.bin-atf-sd NEED_BL32=yes BL32=tee_ls1043ardb.bin SPD=opteed BOOT_MODE=sd   
}

do_install() {
	:
}

OUTPUT_DIR = "${B}/${PLATFORM}/release"
OUTPUT_DIR2 = "${B2}/${PLATFORM}/release"

do_deploy() {
	install -d ${DEPLOYDIR}/atf
        install -m 0644 ${OUTPUT_DIR}/bl2.bin ${DEPLOYDIR}/atf/atf-bl2-nor.bin
        install -m 0644 ${OUTPUT_DIR}/fip.bin ${DEPLOYDIR}/atf/atf-fip-nor.bin
	mkimage -n ${S}/plat/layerscape/board/ls1043/ls1043ardb_rcw_sd.cfg -R ${S}/plat/layerscape/board/ls1043/ls1043ardb_pbi.cfg  -A arm -T pblimage  \
		-a 0x10000000  \
		-d ${OUTPUT_DIR2}/bl2.bin ${OUTPUT_DIR2}/bl2-sd.pbl
       install -m 0644 ${OUTPUT_DIR2}/bl2-sd.pbl ${DEPLOYDIR}/atf/atf-bl2-sd.bin
       install -m 0644 ${OUTPUT_DIR2}/fip.bin ${DEPLOYDIR}/atf/atf-fip-sd.bin

}
addtask deploy before do_build after do_compile
BBCLASSEXTEND = "native nativesdk"
