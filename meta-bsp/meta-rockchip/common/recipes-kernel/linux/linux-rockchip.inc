# Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

require recipes-kernel/linux/linux-yocto.inc

DEPENDS += "openssl-native"

LINUX_VERSION_EXTENSION ?= "-rockchip-${LINUX_KERNEL_TYPE}"
PV = "${LINUX_VERSION}+git${SRCPV}"

KCONFIG_MODE ?= "--alldefconfig"

# Hack for rockchip style images
ROCKCHIP_KERNEL_IMAGES = "kernel.img resource.img"
KERNEL_IMAGETYPES += "${ROCKCHIP_KERNEL_IMAGES}"
python () {
    if not d.getVar('KERNEL_DEVICETREE'):
        raise bb.parse.SkipPackage('KERNEL_DEVICETREE is not specified!')

    # Add rockchip stype image make target, which is '<dts(w/o suffix)>.img'
    d.appendVar('KERNEL_IMAGETYPE_FOR_MAKE', ' ' + d.getVar('KERNEL_DEVICETREE').replace('rockchip/', '').replace('.dtb', '.img'));

    # Remove the virtual make targets
    d.setVar('KERNEL_IMAGETYPE_FOR_MAKE', d.getVar('KERNEL_IMAGETYPE_FOR_MAKE').replace(d.getVar('ROCKCHIP_KERNEL_IMAGES'), ''))
}

# Link rockchip style images
do_install_prepend() {
    for image in ${ROCKCHIP_KERNEL_IMAGES};do
        ln -sf ${B}/${image} ${B}/arch/${ARCH}/boot/
    done
}

# Remove installed kernel images
do_install_append() {
    for type in ${KERNEL_IMAGETYPES} ; do
        rm -f ${D}/${KERNEL_IMAGEDEST}/${type}*
    done
}
