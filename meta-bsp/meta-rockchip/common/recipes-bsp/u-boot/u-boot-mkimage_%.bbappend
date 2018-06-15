# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI = " \
    git://github.com/rockchip-linux/u-boot.git;branch=release \
    file://0001-mkimage-rkcommon-Add-rk3326.patch \
    file://0001-mkimage-rkcommon-Add-rk3308.patch \
"
SRCREV = "328b7e77a21a34b046eaf9a7cf41c7d7c1a82047"
