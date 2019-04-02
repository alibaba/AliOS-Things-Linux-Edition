FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto_${LINUX_VERSION}:"

COMPATIBLE_MACHINE = "raspberrypi3|raspberrypi3-64"

SRC_URI += "${@bb.utils.contains('LINUX_VERSION', '4.9.155', ' git://git.yoctoproject.org/linux-yocto-4.9.git;name=machine;branch=${KBRANCH}; \
           file://0001-merge-from-rasp.patch \
		   file://0001-Revert-efi-libstub-arm64-Set-fpie.patch \
           file://0003-LED-fix-build-error.patch \
           file://0004-MMC-fix-build-error.patch \
           file://0005-FB-fix-build-error.patch \
           file://0006-fix-build-error.patch \
           file://0007-DTS-provide-dtb.patch \
           file://0010-add-CONFIGS-for-LTP-tests.patch \
           file://0011-modify-bcm2709-defconfig.patch \
           file://0012-update-dtc-files.patch \ 
           file://0013-cpu-freq-default-gov-ondemand.patch ', '', d)}"
		   
SRC_URI += "${@bb.utils.contains('LINUX_VERSION', '4.9.49', ' git://git.yoctoproject.org/linux-yocto-4.9.git;name=machine;branch=${KBRANCH}; \
           file://0001-merge-from-rasp.patch \
           file://0002-update-deconfig.patch \
           file://0003-LED-fix-build-error.patch \
           file://0004-MMC-fix-build-error.patch \
           file://0005-FB-fix-build-error.patch \
           file://0006-fix-build-error.patch \
           file://0007-DTS-provide-dtb.patch \
           file://0008-rm-arch-arm64-boot-dts.patch \
           file://0009-provide-fake-makefile-for-dts.patch \
           file://0010-add-CONFIGS-for-LTP-tests.patch \
           file://0011-modify-bcm2709-defconfig.patch \
           file://0012-update-dtc-files.patch \
           file://0013-cpu-freq-default-gov-ondemand.patch ', '', d)}"

KBUILD_DEFCONFIG_raspberrypi3-64 = "defconfig"
KBUILD_DEFCONFIG_raspberrypi3 = "bcm2835_defconfig"
KERNEL_EXTRA_FEATURES = ""

do_rasp_prebuild() {
    echo ${B}
    if [ ${ARCH} = "arm" ]
    then
        mkdir -p ${B}/arch/arm/boot
        cp ${S}/arch/arm/configs/bcm2709_defconfig ${B}/.config
        cp -rf ${S}/dts ${B}/arch/arm/boot/
    else
        mkdir -p ${B}/arch/arm64/boot
        cp ${S}/arch/arm64/configs/raspberrypi3_64_config ${B}/.config
        cp -fr ${S}/dts ${B}/arch/arm64/boot/
    fi
}

addtask rasp_prebuild before do_compile after do_kernel_configcheck

require linux-raspberrypi.inc

# A LOADADDR is needed when building a uImage format kernel. This value is not
# set by default in rpi-4.8.y and later branches so we need to provide it
# manually. This value unused if KERNEL_IMAGETYPE is not uImage.
KERNEL_EXTRA_ARGS += "LOADADDR=0x00008000"
