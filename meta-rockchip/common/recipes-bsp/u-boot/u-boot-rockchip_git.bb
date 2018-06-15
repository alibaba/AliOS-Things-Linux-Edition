# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

DEFAULT_PREFERENCE = "-1"

include u-boot-rockchip.inc

SRC_URI = " \
    git://github.com/rockchip-linux/u-boot.git;branch=release; \
"
# SRCREV = "${AUTOREV}"
SRCREV = "328b7e77a21a34b046eaf9a7cf41c7d7c1a82047"
S = "${WORKDIR}/git"
