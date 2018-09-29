# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

DEFAULT_PREFERENCE = "-1"

include u-boot-rockchip.inc

SRC_URI = " \
    git://github.com/rockchip-linux/u-boot.git;branch=29mirror; \
    file://0001-envtools-make-sure-version-timestamp-header-file-are.patch \
    file://0002-configs-evb-aarch32-rk3308-Extern-boot-delay.patch \
    file://0003-boot_rkimg-Support-boot-mode-without-misc-partition.patch \
    file://0004-configs-rockchip-Add-CONFIG_ENV_SIZE.patch \
    file://0005-fdt_support-Honor-loader-s-bootargs.patch \
    file://0006-env-Add-block-device-env.patch \
    file://0007-env-blk-Add-fallback-blk-devs.patch \
"

SRCREV = "404256445eb51fc76a0107d1b954283851b5150d"
S = "${WORKDIR}/git"
