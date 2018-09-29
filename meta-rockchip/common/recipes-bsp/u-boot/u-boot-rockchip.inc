# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Copyright (C) 2017 Trevor Woerner <twoerner@gmail.com>
# Released under the MIT license (see COPYING.MIT for the terms)

require recipes-bsp/u-boot/u-boot.inc
require ${@oe.utils.all_distro_features(d, 'uota', 'recipes-bootloader/u-boot/u-boot-mender.inc')}

PROVIDES += " u-boot"
RPROVIDES_${PN} += " u-boot"

DESCRIPTION = "Rockchip next-dev U-Boot"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
COMPATIBLE_MACHINE = ".*(rk3308|rk3326).*"

DEPENDS = "dtc-native bc-native swig-native rk-binary-native"

SRC_URI_append = "${@bb.utils.contains('DISTRO_FEATURES', 'uota', \
		  ' file://0001-config_mender-Support-env-blk.patch', \
		  '', \
		  d)}"

do_configure_prepend () {
	if ${@bb.utils.contains('DISTRO_FEATURES', 'uota', 'true', 'false', d)};then
		sed -i "s/CONFIG_ENV_IS_IN_MMC/CONFIG_ENV_IS_IN_BLK/" \
			${S}/configs/${UBOOT_MACHINE}
	fi
}

# Generate rockchip style u-boot binary
UBOOT_BINARY = "u-boot-rk.img"
do_compile_append () {
        UBOOT_TEXT_BASE=`grep -w "CONFIG_SYS_TEXT_BASE" ${B}/include/autoconf.mk`
        loaderimage --pack --uboot ${B}/u-boot.bin ${B}/${UBOOT_BINARY} ${UBOOT_TEXT_BASE#*=} --size 1024 1
}
