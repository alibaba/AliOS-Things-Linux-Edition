# Copyright (c) 2018, Fuzhou Rockchip Electronics Co., Ltd
# Released under the MIT license (see COPYING.MIT for the terms)

require conf/machine/include/soc-family.inc

PREFERRED_PROVIDER_virtual/kernel ?= "linux-rockchip"
PREFERRED_VERSION_linux-rockchip ?= "4.4%"
LINUXLIBCVERSION ?= "4.4%"

MACHINE_EXTRA_RRECOMMENDS_append = " kernel-modules linux-firmware"

SERIAL_CONSOLES ?= "115200;ttyFIQ0"
KERNEL_IMAGETYPE ?= "Image"
KBUILD_DEFCONFIG ?= "rockchip_linux_defconfig"

PREFERRED_PROVIDER_virtual/bootloader ?= "u-boot-rockchip"

IMAGE_FSTYPES += "ext4"
IMAGE_FSTYPES_remove = "iso live"

RK_MINILOADER_INI ?= "${@d.getVar('SOC_FAMILY').upper() + 'MINIALL.ini'}"

IMAGE_FSTYPES += "wic"
WKS_FILE ?= "generic-gptdisk.wks"
do_image_wic[depends] += " \
        virtual/kernel:do_deploy \
        virtual/bootloader:do_deploy \
        rk-binary-loader:do_deploy \
"
export RK_ROOTDEV_UUID ?= "614e0000-0000-4b53-8000-1d28000054a9"
