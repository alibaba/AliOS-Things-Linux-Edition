# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

require linux-rockchip.inc

FILESEXTRAPATHS_prepend := "${FILESEXTRAPATH}:"
FILESOVERRIDES_prepend := "${SOC_FAMILY}-${TRANSLATED_TARGET_ARCH}:"

SRC_URI += " \
	file://defconfig \
"

python () {
    d.delVar('KBUILD_DEFCONFIG')

    # Add all patches
    files = os.listdir(d.getVar('FILESEXTRAPATH'))
    files.sort()
    for file in files:
      if file.endswith('.patch'):
        d.appendVar('SRC_URI', ' file://' + file)
}

COMPATIBLE_MACHINE = ".*(rk3308).*"
