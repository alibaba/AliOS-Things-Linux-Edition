# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI = " \
    git://github.com/rockchip-linux/u-boot.git;branch=29mirror; \
    file://0001-mkimage-rkcommon-Add-rk3326.patch \
    file://0001-mkimage-rkcommon-Add-rk3308.patch \
"
SRCREV = "bcf9093629cab1bad23d1df024aa508ac0797803"
