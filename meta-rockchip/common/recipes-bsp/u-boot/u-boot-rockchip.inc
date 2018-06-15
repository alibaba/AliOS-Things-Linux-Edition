# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Copyright (C) 2017 Trevor Woerner <twoerner@gmail.com>
# Released under the MIT license (see COPYING.MIT for the terms)

require recipes-bsp/u-boot/u-boot.inc

DESCRIPTION = "Rockchip next-dev U-Boot"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
COMPATIBLE_MACHINE = ".*(rk3308|rk3326).*"

DEPENDS = "dtc-native bc-native swig-native rk-binary-native"

# Generate rockchip style u-boot binary
UBOOT_BINARY = "u-boot-rk.img"
do_compile_append () {
        UBOOT_TEXT_BASE=`grep -w "CONFIG_SYS_TEXT_BASE" ${B}/include/autoconf.mk`
        loaderimage --pack --uboot ${B}/u-boot.bin ${B}/${UBOOT_BINARY} ${UBOOT_TEXT_BASE#*=}
}
