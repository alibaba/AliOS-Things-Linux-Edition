# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

require recipes-kernel/linux/linux-yocto.inc
require linux-rockchip.inc

SRC_URI = " \
    git://github.com/rockchip-linux/kernel.git;branch=release-4.4; \
    file://0001-Fix-yocto-compile-error.patch \
    file://0001-arm64-dts-rk3326-Use-UUID-for-rootdev.patch \
    file://0001-arm64-dts-rk3308-Allow-root-fstype-other-than-squash.patch \
"

SRCREV = "3dd9af3221d2a4ea4caf2865bac5fe9aaf2e2643"
LINUX_VERSION = "4.4.126"

SRC_URI_append_rockchip-rk3308-evb += " file://cgroups.cfg file://ext4.cfg"

COMPATIBLE_MACHINE = ".*(rk3308|rk3326).*"
