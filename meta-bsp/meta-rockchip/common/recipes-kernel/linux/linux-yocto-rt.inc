# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

require linux-rockchip.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/files/linux-yocto_${LINUX_VERSION}:${THISDIR}/files/linux-yocto-rt_${LINUX_VERSION}:"
FILESOVERRIDES_prepend := "rk3308-arm:"

SRC_URI += " \
        file://defconfig \
        file://0001-add-rk3308-linux-rt-defconfig.patch \
        file://0002-disable-preempt-in-cpu-pm-enter-exit.patch \
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

do_rk3308_rt_prebuild() {
        cp ${S}/arch/arm/configs/rk3308_linux_rt_defconfig ${B}/.config
}

addtask rk3308_rt_prebuild before do_compile after do_kernel_configcheck

COMPATIBLE_MACHINE = ".*(rk3308).*"
